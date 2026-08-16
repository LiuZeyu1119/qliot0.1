#include "gucds/core/pcanbasicsession.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLibrary>
#include <QThread>
#include <QTimer>

#include <algorithm>
#include <array>
#include <cstring>
#include <utility>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#define GUCDS_PCAN_CALL WINAPI
#else
#define GUCDS_PCAN_CALL
#endif

namespace {

constexpr quint32 kPcanErrorOk = 0x00000U;
constexpr quint32 kPcanErrorBusLight = 0x00004U;
constexpr quint32 kPcanErrorBusHeavy = 0x00008U;
constexpr quint32 kPcanErrorBusOff = 0x00010U;
constexpr quint32 kPcanErrorQueueEmpty = 0x00020U;
constexpr quint32 kPcanErrorNoDriver = 0x00200U;
constexpr quint32 kPcanErrorInvalidHandle = 0x01C00U;
constexpr quint32 kPcanErrorBusPassive = 0x40000U;
constexpr quint32 kPcanErrorCaution = 0x2000000U;
constexpr quint32 kPcanErrorNotInitialized = 0x4000000U;
constexpr quint32 kPcanErrorInvalidOperation = 0x8000000U;
constexpr quint32 kPcanAnyBusError =
    kPcanErrorBusLight | kPcanErrorBusHeavy | kPcanErrorBusOff | kPcanErrorBusPassive;

constexpr quint8 kPcanMessageRemote = 0x01U;
constexpr quint8 kPcanMessageExtended = 0x02U;
constexpr quint8 kPcanMessageError = 0x40U;
constexpr quint8 kPcanMessageStatus = 0x80U;
constexpr quint8 kPcanBusSpeedNominalParameter = 0x1AU;

struct PcanMessage
{
    quint32 id;
    quint8 type;
    quint8 length;
    quint8 data[8];
};

struct PcanTimestamp
{
    quint32 millis;
    quint16 millisOverflow;
    quint16 micros;
};

static_assert(sizeof(PcanMessage) == 16, "PCAN-Basic classic CAN message ABI mismatch");
static_assert(sizeof(PcanTimestamp) == 8, "PCAN-Basic timestamp ABI mismatch");

template<typename Function>
bool resolveFunction(QLibrary &library, const char *name, Function &function)
{
    function = reinterpret_cast<Function>(library.resolve(name));
    return function != nullptr;
}

QString systemPcanLibraryPath()
{
#ifdef Q_OS_WIN
    std::array<wchar_t, MAX_PATH + 1> path{};
    const UINT length = GetSystemDirectoryW(path.data(), static_cast<UINT>(path.size()));
    if (length > 0 && length < path.size())
        return QDir(QString::fromWCharArray(path.data(), static_cast<qsizetype>(length)))
            .filePath(QStringLiteral("PCANBasic.dll"));
#endif
    return {};
}

bool isTerminalStatus(quint32 status)
{
    return (status & (kPcanErrorNoDriver | kPcanErrorInvalidHandle
                      | kPcanErrorNotInitialized | kPcanErrorInvalidOperation)) != 0U;
}

constexpr quint32 decodeStatusFrame(const quint8 *data)
{
    return (quint32(data[0]) << 24U) | (quint32(data[1]) << 16U)
        | (quint32(data[2]) << 8U) | quint32(data[3]);
}

constexpr quint8 kStatusDecodeTest[4] = {0x12U, 0x34U, 0x56U, 0x78U};
static_assert(decodeStatusFrame(kStatusDecodeTest) == 0x12345678U,
              "PCAN status-frame byte order mismatch");

} // namespace

namespace gucds {

struct PcanBasicSession::Api
{
    using Initialize = quint32 (GUCDS_PCAN_CALL *)(quint16, quint16, quint8, quint32, quint16);
    using Uninitialize = quint32 (GUCDS_PCAN_CALL *)(quint16);
    using Read = quint32 (GUCDS_PCAN_CALL *)(quint16, PcanMessage *, PcanTimestamp *);
    using Write = quint32 (GUCDS_PCAN_CALL *)(quint16, PcanMessage *);
    using GetStatus = quint32 (GUCDS_PCAN_CALL *)(quint16);
    using GetValue = quint32 (GUCDS_PCAN_CALL *)(quint16, quint8, void *, quint32);
    using GetErrorText = quint32 (GUCDS_PCAN_CALL *)(quint32, quint16, char *);

    QLibrary library;
    Initialize initialize = nullptr;
    Uninitialize uninitialize = nullptr;
    Read read = nullptr;
    Write write = nullptr;
    GetStatus getStatus = nullptr;
    GetValue getValue = nullptr;
    GetErrorText getErrorText = nullptr;
};

PcanBasicSession::PcanBasicSession(QObject *parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
{
    qRegisterMetaType<gucds::CanFrame>();
    m_pollTimer->setInterval(10);
    m_pollTimer->setTimerType(Qt::PreciseTimer);
    connect(m_pollTimer, &QTimer::timeout, this, &PcanBasicSession::poll);
}

PcanBasicSession::~PcanBasicSession()
{
    close();
}

bool PcanBasicSession::open(int usbChannel, int bitrate)
{
    Q_ASSERT(QThread::currentThread() == thread());
    close();
    m_lastError.clear();

    const quint16 handle = usbChannelHandle(usbChannel);
    const quint16 rate = baudCode(bitrate);
    if (handle == 0) {
        setError(tr("PCAN-USB 通道必须在 1 到 16 之间"));
        return false;
    }
    if (rate == 0) {
        setError(tr("不支持的 CAN 波特率：%1 bit/s").arg(bitrate));
        return false;
    }
    if (!loadApi())
        return false;

    const quint32 status = m_api->initialize(handle, rate, 0, 0, 0);
    if (status != kPcanErrorOk && status != kPcanErrorCaution) {
        setError(tr("打开 PCAN-USB 失败：%1").arg(statusDescription(status)));
        return false;
    }

    if (m_api->getValue) {
        quint32 actualBitrate = 0;
        const quint32 valueStatus = m_api->getValue(
            handle, kPcanBusSpeedNominalParameter, &actualBitrate, sizeof(actualBitrate));
        if (valueStatus == kPcanErrorOk && actualBitrate != static_cast<quint32>(bitrate)) {
            m_api->uninitialize(handle);
            setError(tr("PCAN 通道实际波特率为 %1 bit/s，与请求的 %2 bit/s 不一致")
                         .arg(actualBitrate)
                         .arg(bitrate));
            return false;
        }
    }

    m_handle = handle;
    m_usbChannel = usbChannel;
    m_bitrate = bitrate;
    m_open = true;
    m_lastBusStatus = m_api->getStatus(m_handle);
    m_lastReadError = kPcanErrorOk;
    m_pollTimer->start();
    emit connectionChanged(true);
    if (!m_open)
        return false;
    emit busStatusChanged(m_lastBusStatus, statusDescription(m_lastBusStatus));
    if (status == kPcanErrorCaution)
        emit errorOccurred(tr("PCAN 通道已打开，但驱动报告警告：%1").arg(statusDescription(status)));
    return m_open;
}

void PcanBasicSession::close()
{
    Q_ASSERT(QThread::currentThread() == thread());
    m_pollTimer->stop();
    if (!m_open)
        return;

    const quint32 status = m_api && m_api->uninitialize
        ? m_api->uninitialize(m_handle)
        : kPcanErrorNotInitialized;
    m_open = false;
    m_handle = 0;
    m_usbChannel = 0;
    m_bitrate = 0;
    m_lastReadError = kPcanErrorOk;
    emit connectionChanged(false);
    if (status != kPcanErrorOk)
        setError(tr("关闭 PCAN 通道时驱动返回：%1").arg(statusDescription(status)));
}

bool PcanBasicSession::isOpen() const
{
    return m_open;
}

int PcanBasicSession::usbChannel() const
{
    return m_usbChannel;
}

int PcanBasicSession::bitrate() const
{
    return m_bitrate;
}

QString PcanBasicSession::apiPath() const
{
    return m_apiPath;
}

QString PcanBasicSession::lastError() const
{
    return m_lastError;
}

bool PcanBasicSession::sendFrame(const CanFrame &frame)
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_open || !m_api || !m_api->write) {
        setError(tr("PCAN 通道尚未连接"));
        return false;
    }

    QString validationError;
    if (!validateFrame(frame, &validationError)) {
        setError(validationError);
        return false;
    }

    PcanMessage message{};
    message.id = frame.id;
    message.type = (frame.extended ? kPcanMessageExtended : 0U)
        | (frame.remote ? kPcanMessageRemote : 0U);
    message.length = frame.remote ? frame.dlc : static_cast<quint8>(frame.payload.size());
    if (!frame.remote && !frame.payload.isEmpty())
        std::memcpy(message.data, frame.payload.constData(), static_cast<size_t>(frame.payload.size()));

    const quint32 status = m_api->write(m_handle, &message);
    if (status != kPcanErrorOk && status != kPcanErrorCaution) {
        setError(tr("CAN 帧发送失败：%1").arg(statusDescription(status)));
        return false;
    }

    CanFrame sent = frame;
    sent.dlc = message.length;
    sent.transmitted = true;
    sent.hardwareTimestampUs = 0;
    sent.wallClockMs = QDateTime::currentMSecsSinceEpoch();
    m_lastError.clear();
    emit frameSent(sent);
    if (status == kPcanErrorCaution)
        emit errorOccurred(tr("CAN 帧已提交，但驱动报告警告：%1").arg(statusDescription(status)));
    return true;
}

quint32 PcanBasicSession::busStatus() const
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_open || !m_api || !m_api->getStatus)
        return kPcanErrorNotInitialized;
    return m_api->getStatus(m_handle);
}

quint16 PcanBasicSession::baudCode(int bitrate)
{
    switch (bitrate) {
    case 1000000: return 0x0014U;
    case 800000: return 0x0016U;
    case 500000: return 0x001CU;
    case 250000: return 0x011CU;
    case 125000: return 0x031CU;
    case 100000: return 0x432FU;
    case 50000: return 0x472FU;
    case 20000: return 0x532FU;
    case 10000: return 0x672FU;
    case 5000: return 0x7F7FU;
    default: return 0;
    }
}

quint16 PcanBasicSession::usbChannelHandle(int usbChannel)
{
    if (usbChannel >= 1 && usbChannel <= 8)
        return static_cast<quint16>(0x50U + usbChannel);
    if (usbChannel >= 9 && usbChannel <= 16)
        return static_cast<quint16>(0x500U + usbChannel);
    return 0;
}

bool PcanBasicSession::validateFrame(const CanFrame &frame, QString *errorMessage)
{
    if (errorMessage)
        errorMessage->clear();
    auto fail = [errorMessage](const QString &message) {
        if (errorMessage)
            *errorMessage = message;
        return false;
    };

    auto translated = [](const char *source) {
        return QCoreApplication::translate("gucds::PcanBasicSession", source);
    };

    const quint32 maximumId = frame.extended ? 0x1FFFFFFFU : 0x7FFU;
    if (frame.id > maximumId)
        return fail(translated("CAN ID 超出%1帧范围")
                        .arg(frame.extended ? translated("扩展") : translated("标准")));
    if (frame.dlc > 8U)
        return fail(translated("CAN DLC 不能超过 8"));
    if (frame.remote) {
        if (!frame.payload.isEmpty())
            return fail(translated("RTR 远程帧不能携带数据"));
    } else {
        if (frame.payload.size() > 8)
            return fail(translated("经典 CAN 数据不能超过 8 字节"));
        if (frame.dlc != 0U && frame.dlc != frame.payload.size())
            return fail(translated("CAN DLC 与数据长度不一致"));
    }
    return true;
}

bool PcanBasicSession::hasBusError(quint32 status)
{
    return (status & kPcanAnyBusError) != 0U;
}

void PcanBasicSession::poll()
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_open || !m_api)
        return;

    const quint16 activeHandle = m_handle;
    constexpr int maximumFramesPerPoll = 512;
    for (int count = 0; count < maximumFramesPerPoll; ++count) {
        PcanMessage message{};
        PcanTimestamp timestamp{};
        const quint32 status = m_api->read(activeHandle, &message, &timestamp);
        if (status == kPcanErrorQueueEmpty) {
            m_lastReadError = kPcanErrorOk;
            break;
        }
        if ((message.type & kPcanMessageStatus) != 0U) {
            // PCAN-Basic stores a status frame's TPCANStatus in DATA[0..3]
            // in network byte order. ID and LEN are explicitly undefined.
            quint32 frameStatus = decodeStatusFrame(message.data);
            if (frameStatus == kPcanErrorOk && status != kPcanErrorOk)
                frameStatus = status;
            m_lastReadError = kPcanErrorOk;
            m_lastBusStatus = frameStatus;
            const QString description = statusDescription(frameStatus);
            emit busStatusChanged(frameStatus, description);
            if (frameStatus != kPcanErrorOk)
                emit errorOccurred(tr("收到 PCAN 状态帧：%1").arg(description));
            if (!m_open || m_handle != activeHandle)
                return;
            if (isTerminalStatus(frameStatus)) {
                m_pollTimer->stop();
                m_api->uninitialize(activeHandle);
                m_open = false;
                m_handle = 0;
                m_usbChannel = 0;
                m_bitrate = 0;
                emit connectionChanged(false);
                return;
            }
            continue;
        }
        if (status != kPcanErrorOk) {
            if (status != m_lastReadError) {
                m_lastReadError = status;
                setError(tr("读取 PCAN 接收队列失败：%1").arg(statusDescription(status)));
            }
            if (!m_open || m_handle != activeHandle)
                return;
            if (isTerminalStatus(status)) {
                m_pollTimer->stop();
                m_api->uninitialize(activeHandle);
                m_open = false;
                m_handle = 0;
                m_usbChannel = 0;
                m_bitrate = 0;
                emit connectionChanged(false);
                return;
            }
            break;
        }
        m_lastReadError = kPcanErrorOk;
        if ((message.type & kPcanMessageError) != 0U) {
            emit errorOccurred(tr("收到 CAN 错误帧（ID 0x%1）")
                                   .arg(message.id, 8, 16, QLatin1Char('0')));
            if (!m_open || m_handle != activeHandle)
                return;
            continue;
        }
        if (message.length > 8U) {
            emit errorOccurred(tr("驱动返回了无效 CAN DLC：%1").arg(message.length));
            continue;
        }

        CanFrame frame;
        frame.id = message.id;
        frame.extended = (message.type & kPcanMessageExtended) != 0U;
        frame.remote = (message.type & kPcanMessageRemote) != 0U;
        frame.dlc = message.length;
        if (!frame.remote)
            frame.payload = QByteArray(reinterpret_cast<const char *>(message.data), message.length);
        frame.hardwareTimestampUs = timestamp.micros
            + 1000ULL * timestamp.millis
            + (1ULL << 32) * 1000ULL * timestamp.millisOverflow;
        frame.wallClockMs = QDateTime::currentMSecsSinceEpoch();
        emit frameReceived(frame);
        if (!m_open || m_handle != activeHandle)
            return;
    }

    if (!m_open || m_handle != activeHandle)
        return;
    const quint32 status = m_api->getStatus(activeHandle);
    if (status != m_lastBusStatus) {
        m_lastBusStatus = status;
        const QString description = statusDescription(status);
        emit busStatusChanged(status, description);
        if (hasBusError(status))
            emit errorOccurred(tr("CAN 总线错误：%1").arg(description));
    }
}

bool PcanBasicSession::loadApi()
{
#ifndef Q_OS_WIN
    setError(tr("PCAN-Basic 后端当前仅支持 Windows"));
    return false;
#else
    if (m_api && m_api->library.isLoaded())
        return true;

    m_api = std::make_unique<Api>();
    QStringList candidates;
    const QString applicationLibrary = QDir(QCoreApplication::applicationDirPath())
                                           .filePath(QStringLiteral("PCANBasic.dll"));
    if (QFileInfo::exists(applicationLibrary))
        candidates.append(QFileInfo(applicationLibrary).absoluteFilePath());
    const QString systemLibrary = systemPcanLibraryPath();
    if (!systemLibrary.isEmpty() && QFileInfo::exists(systemLibrary))
        candidates.append(QFileInfo(systemLibrary).absoluteFilePath());

    if (candidates.isEmpty()) {
        setError(tr("未找到 PCANBasic.dll；请安装 PCAN-Basic x64 驱动组件"));
        return false;
    }

    QStringList loadErrors;
    for (const QString &candidate : std::as_const(candidates)) {
        m_api->library.setFileName(candidate);
        if (!m_api->library.load()) {
            loadErrors.append(QStringLiteral("%1: %2").arg(candidate, m_api->library.errorString()));
            continue;
        }

        const bool complete = resolveFunction(m_api->library, "CAN_Initialize", m_api->initialize)
            && resolveFunction(m_api->library, "CAN_Uninitialize", m_api->uninitialize)
            && resolveFunction(m_api->library, "CAN_Read", m_api->read)
            && resolveFunction(m_api->library, "CAN_Write", m_api->write)
            && resolveFunction(m_api->library, "CAN_GetStatus", m_api->getStatus)
            && resolveFunction(m_api->library, "CAN_GetErrorText", m_api->getErrorText);
        if (complete) {
            resolveFunction(m_api->library, "CAN_GetValue", m_api->getValue);
            m_apiPath = candidate;
            return true;
        }

        loadErrors.append(tr("%1: 缺少必要的 PCAN-Basic 导出函数").arg(candidate));
        m_api->library.unload();
    }

    setError(tr("无法加载 PCAN-Basic API：%1").arg(loadErrors.join(QStringLiteral("; "))));
    return false;
#endif
}

void PcanBasicSession::setError(const QString &message)
{
    m_lastError = message;
    emit errorOccurred(message);
}

QString PcanBasicSession::statusDescription(quint32 status) const
{
    if (m_api && m_api->getErrorText) {
        std::array<char, 256> text{};
        if (m_api->getErrorText(status, 0x09U, text.data()) == kPcanErrorOk && text[0] != '\0')
            return QString::fromLocal8Bit(text.data());
    }
    return QStringLiteral("PCAN status 0x%1").arg(status, 8, 16, QLatin1Char('0')).toUpper();
}

} // namespace gucds

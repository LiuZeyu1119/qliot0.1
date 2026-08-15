#include "gucds/core/serialsession.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QThread>

#include <algorithm>
#include <cstdint>
#include <string>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#ifdef Q_OS_UNIX
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace {

QString serialText(const char *source)
{
    return QCoreApplication::translate("SerialSession", source);
}

void setError(QString *error, const QString &message)
{
    if (error)
        *error = message;
}

#ifdef Q_OS_WIN

HANDLE serialHandle(void *handle)
{
    return static_cast<HANDLE>(handle);
}

QString windowsErrorMessage(const QString &action, DWORD code)
{
    wchar_t *buffer = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER
        | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD length = FormatMessageW(flags,
                                        nullptr,
                                        code,
                                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                        reinterpret_cast<wchar_t *>(&buffer),
                                        0,
                                        nullptr);

    QString message;
    if (length > 0 && buffer)
        message = QString::fromWCharArray(buffer, int(length)).trimmed();
    if (buffer)
        LocalFree(buffer);
    if (message.isEmpty())
        message = QStringLiteral("Windows error 0x%1").arg(code, 0, 16);

    return serialText("%1失败：%2").arg(action, message);
}

QString serialDevicePath(const QString &portName)
{
    if (portName.startsWith(QStringLiteral("\\\\.\\")))
        return portName;
    return QStringLiteral("\\\\.\\%1").arg(portName);
}

bool configureSerialPort(HANDLE handle, int baudRate, QString *error)
{
    DCB dcb = {};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(handle, &dcb)) {
        setError(error, windowsErrorMessage(serialText("读取串口参数"), GetLastError()));
        return false;
    }

    dcb.BaudRate = DWORD(baudRate);
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fAbortOnError = FALSE;

    if (!SetCommState(handle, &dcb)) {
        setError(error, windowsErrorMessage(serialText("配置串口参数"), GetLastError()));
        return false;
    }

    COMMTIMEOUTS timeouts = {};
    timeouts.ReadIntervalTimeout = 20;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 1000;
    if (!SetCommTimeouts(handle, &timeouts)) {
        setError(error, windowsErrorMessage(serialText("配置串口超时"), GetLastError()));
        return false;
    }

    SetupComm(handle, 4096, 4096);
    PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    return true;
}

#endif

#ifdef Q_OS_UNIX

int serialFd(void *handle)
{
    return int(reinterpret_cast<intptr_t>(handle)) - 1;
}

QString posixErrorMessage(const QString &action)
{
    return serialText("%1失败：%2").arg(action, QString::fromLocal8Bit(std::strerror(errno)));
}

speed_t baudConstant(int baudRate)
{
    switch (baudRate) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
#ifdef B230400
    case 230400:
        return B230400;
#endif
#ifdef B460800
    case 460800:
        return B460800;
#endif
    default:
        return speed_t(0);
    }
}

bool configureSerialPort(int fd, int baudRate, QString *error)
{
    const speed_t speed = baudConstant(baudRate);
    if (speed == speed_t(0)) {
        setError(error, serialText("当前 POSIX 平台不支持波特率 %1").arg(baudRate));
        return false;
    }

    termios options = {};
    if (tcgetattr(fd, &options) != 0) {
        setError(error, posixErrorMessage(serialText("读取串口参数")));
        return false;
    }
    cfmakeraw(&options);
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);
    options.c_cflag |= CLOCAL | CREAD;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 1;
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        setError(error, posixErrorMessage(serialText("配置串口参数")));
        return false;
    }
    tcflush(fd, TCIOFLUSH);
    return true;
}

#endif

} // namespace

namespace gucds {

SerialSession::SerialSession() = default;

SerialSession::~SerialSession()
{
    close();
}

QStringList SerialSession::availablePorts()
{
    QStringList ports;

#ifdef Q_OS_WIN
    wchar_t target[1024] = {};
    for (int index = 1; index <= 256; ++index) {
        const QString name = QStringLiteral("COM%1").arg(index);
        const std::wstring wideName = name.toStdWString();
        if (QueryDosDeviceW(wideName.c_str(), target, DWORD(sizeof(target) / sizeof(target[0]))) != 0)
            ports.append(name);
    }
#endif

#ifdef Q_OS_UNIX
    QDir dev(QStringLiteral("/dev"));
    const QStringList filters = {
        QStringLiteral("ttyUSB*"),
        QStringLiteral("ttyACM*"),
        QStringLiteral("ttyS*"),
        QStringLiteral("cu.*"),
        QStringLiteral("tty.*"),
    };
    for (const QString &entry : dev.entryList(filters, QDir::System | QDir::Files, QDir::Name))
        ports.append(dev.absoluteFilePath(entry));
#endif

    ports.removeDuplicates();
    std::sort(ports.begin(), ports.end(), [](const QString &left, const QString &right) {
        const bool leftCom = left.startsWith(QStringLiteral("COM"), Qt::CaseInsensitive);
        const bool rightCom = right.startsWith(QStringLiteral("COM"), Qt::CaseInsensitive);
        bool leftOk = false;
        bool rightOk = false;
        const int leftNumber = left.mid(3).toInt(&leftOk);
        const int rightNumber = right.mid(3).toInt(&rightOk);
        if (leftCom && rightCom && leftOk && rightOk)
            return leftNumber < rightNumber;
        return QString::localeAwareCompare(left, right) < 0;
    });
    return ports;
}

bool SerialSession::open(const QString &portName, int baudRate, QString *error)
{
    const QString trimmedPort = portName.trimmed();
    if (trimmedPort.isEmpty()) {
        setError(error, serialText("串口号不能为空"));
        m_connected = false;
        return false;
    }

    if (baudRate <= 0) {
        setError(error, serialText("波特率必须大于0"));
        m_connected = false;
        return false;
    }

    close();
    m_portName = trimmedPort;
    m_baudRate = baudRate;

#ifdef Q_OS_WIN
    const QString path = serialDevicePath(trimmedPort);
    const std::wstring widePath = path.toStdWString();
    HANDLE handle = CreateFileW(widePath.c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                nullptr,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        setError(error, windowsErrorMessage(serialText("打开串口 %1").arg(trimmedPort), GetLastError()));
        m_connected = false;
        return false;
    }

    if (!configureSerialPort(handle, baudRate, error)) {
        CloseHandle(handle);
        m_connected = false;
        return false;
    }

    m_handle = handle;
    m_connected = true;
    setError(error, {});
    return true;
#elif defined(Q_OS_UNIX)
    const QByteArray nativePath = QFile::encodeName(trimmedPort);
    const int fd = ::open(nativePath.constData(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        setError(error, posixErrorMessage(serialText("打开串口 %1").arg(trimmedPort)));
        m_connected = false;
        return false;
    }
    if (!configureSerialPort(fd, baudRate, error)) {
        ::close(fd);
        m_connected = false;
        return false;
    }
    m_handle = reinterpret_cast<void *>(intptr_t(fd + 1));
    m_connected = true;
    setError(error, {});
    return true;
#else
    setError(error, serialText("当前操作系统不支持 POSIX 或 Win32 串口 API"));
    m_connected = false;
    return false;
#endif
}

bool SerialSession::reconnect(QString *error)
{
    return open(m_portName, m_baudRate, error);
}

void SerialSession::close()
{
#ifdef Q_OS_WIN
    if (m_handle && serialHandle(m_handle) != INVALID_HANDLE_VALUE)
        CloseHandle(serialHandle(m_handle));
#elif defined(Q_OS_UNIX)
    if (m_handle)
        ::close(serialFd(m_handle));
#endif
    m_handle = nullptr;
    m_connected = false;
}

bool SerialSession::writeFrame(const QByteArray &frame, QString *error)
{
    if (!m_connected || !m_handle) {
        setError(error, serialText("串口未连接"));
        return false;
    }

    if (frame.isEmpty()) {
        setError(error, serialText("发送帧不能为空"));
        return false;
    }

#ifdef Q_OS_WIN
    DWORD written = 0;
    const DWORD requested = DWORD(frame.size());
    if (!WriteFile(serialHandle(m_handle), frame.constData(), requested, &written, nullptr)) {
        setError(error, windowsErrorMessage(serialText("发送串口数据"), GetLastError()));
        return false;
    }

    if (written != requested) {
        setError(error, serialText("串口发送不完整：%1/%2 字节").arg(written).arg(requested));
        return false;
    }

    FlushFileBuffers(serialHandle(m_handle));
    setError(error, {});
    return true;
#elif defined(Q_OS_UNIX)
    QElapsedTimer timer;
    timer.start();
    qsizetype written = 0;
    while (written < frame.size()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            setError(error, serialText("串口发送已取消"));
            return false;
        }
        const ssize_t chunk = ::write(serialFd(m_handle), frame.constData() + written, size_t(frame.size() - written));
        if (chunk > 0) {
            written += chunk;
            continue;
        }
        if (chunk < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            setError(error, posixErrorMessage(serialText("发送串口数据")));
            return false;
        }
        if (timer.elapsed() >= 1000) {
            setError(error, serialText("串口发送超时：%1/%2 字节").arg(written).arg(frame.size()));
            return false;
        }
        QThread::msleep(2);
    }
    tcdrain(serialFd(m_handle));
    setError(error, {});
    return true;
#else
    Q_UNUSED(frame)
    setError(error, serialText("当前操作系统不支持 POSIX 或 Win32 串口 API"));
    return false;
#endif
}

bool SerialSession::readFrame(int expectedBytes, QByteArray *frame, QString *error, int timeoutMs)
{
    if (!frame) {
        setError(error, serialText("接收缓冲区不能为空"));
        return false;
    }

    frame->clear();
    if (!m_connected || !m_handle) {
        setError(error, serialText("串口未连接"));
        return false;
    }

    if (expectedBytes < 0) {
        setError(error, serialText("期望接收字节数无效"));
        return false;
    }

    if (expectedBytes == 0) {
        setError(error, {});
        return true;
    }

#ifdef Q_OS_WIN
    QByteArray buffer;
    buffer.resize(expectedBytes);

    QElapsedTimer timer;
    timer.start();
    int received = 0;
    while (received < expectedBytes) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            buffer.resize(received);
            *frame = buffer;
            setError(error, serialText("串口读取已取消"));
            return false;
        }
        DWORD chunk = 0;
        const DWORD remaining = DWORD(expectedBytes - received);
        if (!ReadFile(serialHandle(m_handle), buffer.data() + received, remaining, &chunk, nullptr)) {
            setError(error, windowsErrorMessage(serialText("读取串口数据"), GetLastError()));
            return false;
        }

        if (chunk > 0) {
            received += int(chunk);
            continue;
        }

        if (timer.elapsed() >= timeoutMs) {
            buffer.resize(received);
            *frame = buffer;
            setError(error, serialText("串口读取超时：收到 %1/%2 字节").arg(received).arg(expectedBytes));
            return false;
        }
    }

    *frame = buffer;
    setError(error, {});
    return true;
#elif defined(Q_OS_UNIX)
    QByteArray buffer;
    buffer.reserve(expectedBytes);
    QElapsedTimer timer;
    timer.start();
    while (buffer.size() < expectedBytes) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            *frame = buffer;
            setError(error, serialText("串口读取已取消"));
            return false;
        }
        char chunkBuffer[1024];
        const qsizetype remaining = expectedBytes - buffer.size();
        const ssize_t chunk = ::read(serialFd(m_handle), chunkBuffer, size_t((std::min)(remaining, qsizetype(sizeof(chunkBuffer)))));
        if (chunk > 0) {
            buffer.append(chunkBuffer, int(chunk));
            continue;
        }
        if (chunk < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            setError(error, posixErrorMessage(serialText("读取串口数据")));
            return false;
        }
        if (timer.elapsed() >= timeoutMs) {
            *frame = buffer;
            setError(error, serialText("串口读取超时：收到 %1/%2 字节").arg(buffer.size()).arg(expectedBytes));
            return false;
        }
        QThread::msleep(2);
    }
    *frame = buffer;
    setError(error, {});
    return true;
#else
    Q_UNUSED(timeoutMs)
    setError(error, serialText("当前操作系统不支持 POSIX 或 Win32 串口 API"));
    return false;
#endif
}

bool SerialSession::transactFrame(const QByteArray &request,
                                  int expectedResponseBytes,
                                  QByteArray *response,
                                  QString *error,
                                  int timeoutMs)
{
    if (!response) {
        setError(error, serialText("响应缓冲区不能为空"));
        return false;
    }

#ifdef Q_OS_WIN
    if (m_connected && m_handle)
        PurgeComm(serialHandle(m_handle), PURGE_RXCLEAR | PURGE_RXABORT);
#elif defined(Q_OS_UNIX)
    if (m_connected && m_handle)
        tcflush(serialFd(m_handle), TCIFLUSH);
#endif

    if (!writeFrame(request, error))
        return false;
    return readFrame(expectedResponseBytes, response, error, timeoutMs);
}

bool SerialSession::transactText(const QString &request,
                                 QByteArray *response,
                                 QString *error,
                                 int timeoutMs,
                                 int idleTimeoutMs)
{
    if (!response) {
        setError(error, serialText("响应缓冲区不能为空"));
        return false;
    }
    response->clear();
    if (request.trimmed().isEmpty()) {
        setError(error, serialText("AT/配置命令不能为空"));
        return false;
    }
    if (!m_connected || !m_handle) {
        setError(error, serialText("串口未连接"));
        return false;
    }

#ifdef Q_OS_WIN
    PurgeComm(serialHandle(m_handle), PURGE_RXCLEAR | PURGE_RXABORT);
#elif defined(Q_OS_UNIX)
    tcflush(serialFd(m_handle), TCIFLUSH);
#endif

    QByteArray frame = request.trimmed().toUtf8();
    frame.append("\r\n");
    if (!writeFrame(frame, error))
        return false;

    return readUntilIdle(response, error, timeoutMs, idleTimeoutMs);
}

bool SerialSession::readUntilIdle(QByteArray *data,
                                  QString *error,
                                  int timeoutMs,
                                  int idleTimeoutMs)
{
    if (!data) {
        setError(error, serialText("接收缓冲区不能为空"));
        return false;
    }
    data->clear();
    if (!m_connected || !m_handle) {
        setError(error, serialText("串口未连接"));
        return false;
    }
    if (timeoutMs <= 0 || idleTimeoutMs <= 0) {
        setError(error, serialText("串口读取超时参数无效"));
        return false;
    }

    constexpr qsizetype maximumResponseBytes = 1024 * 1024;
    QElapsedTimer totalTimer;
    QElapsedTimer idleTimer;
    totalTimer.start();
    bool receivedAny = false;
    while (totalTimer.elapsed() < timeoutMs) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            setError(error, serialText("串口读取已取消"));
            return false;
        }
        char chunkBuffer[2048];
        int received = 0;
#ifdef Q_OS_WIN
        DWORD chunk = 0;
        if (!ReadFile(serialHandle(m_handle), chunkBuffer, DWORD(sizeof(chunkBuffer)), &chunk, nullptr)) {
            setError(error, windowsErrorMessage(serialText("读取串口文本回包"), GetLastError()));
            return false;
        }
        received = int(chunk);
#elif defined(Q_OS_UNIX)
        const ssize_t chunk = ::read(serialFd(m_handle), chunkBuffer, sizeof(chunkBuffer));
        if (chunk < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            setError(error, posixErrorMessage(serialText("读取串口文本回包")));
            return false;
        }
        received = chunk > 0 ? int(chunk) : 0;
#endif
        if (received > 0) {
            if (data->size() + received > maximumResponseBytes) {
                setError(error, serialText("串口文本回包超过 1 MiB 限制"));
                return false;
            }
            data->append(chunkBuffer, received);
            receivedAny = true;
            idleTimer.restart();
            continue;
        }
        if (receivedAny && idleTimer.elapsed() >= idleTimeoutMs) {
            setError(error, {});
            return true;
        }
        QThread::msleep(2);
    }

    if (receivedAny) {
        setError(error, {});
        return true;
    }
    setError(error, serialText("串口等待数据超时：%1 ms").arg(timeoutMs));
    return false;
}

bool SerialSession::isConnected() const
{
    return m_connected;
}

bool SerialSession::isBackendAvailable() const
{
#if defined(Q_OS_WIN) || defined(Q_OS_UNIX)
    return true;
#else
    return false;
#endif
}

QString SerialSession::portName() const
{
    return m_portName;
}

int SerialSession::baudRate() const
{
    return m_baudRate;
}

QString SerialSession::statusText() const
{
    return m_connected ? serialText("已连接") : serialText("未连接");
}

} // namespace gucds

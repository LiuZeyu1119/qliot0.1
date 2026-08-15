#include "gucds/core/labviewdatabase.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringDecoder>

#include <atomic>
#include <cmath>
#include <utility>

namespace gucds {

namespace {

struct TdmsColumn
{
    QString groupName;
    QString columnName;
    int columnIndex = 0;
    QStringList values;
};

using TdmsGroups = QMap<QString, QMap<QString, QStringList>>;

QString databaseText(const char *source)
{
    return QCoreApplication::translate("LabviewDatabase", source);
}

QString setError(const QString &message, QString *errorMessage)
{
    if (errorMessage)
        *errorMessage = message;
    return message;
}

quint32 readU32(const QByteArray &bytes, qsizetype &offset)
{
    const uchar *p = reinterpret_cast<const uchar *>(bytes.constData() + offset);
    offset += 4;
    return quint32(p[0]) | (quint32(p[1]) << 8) | (quint32(p[2]) << 16) | (quint32(p[3]) << 24);
}

quint64 readU64(const QByteArray &bytes, qsizetype &offset)
{
    quint64 value = 0;
    for (int i = 0; i < 8; ++i)
        value |= quint64(uchar(bytes.at(offset + i))) << (8 * i);
    offset += 8;
    return value;
}

QString readTdmsString(const QByteArray &bytes, qsizetype &offset)
{
    const quint32 length = readU32(bytes, offset);
    const QString value = QString::fromUtf8(bytes.constData() + offset, int(length));
    offset += length;
    return value;
}

QString decodeGb18030(const QByteArray &bytes)
{
    QStringDecoder decoder("GB18030");
    if (!decoder.isValid())
        decoder = QStringDecoder(QStringConverter::System);
    return decoder.decode(bytes);
}

QStringList splitLines(const QString &text)
{
    QString normalized = text;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    normalized.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    QStringList lines = normalized.split(QLatin1Char('\n'));
    while (!lines.isEmpty() && lines.last().isEmpty())
        lines.removeLast();
    return lines;
}

QStringList readGbTable(const QString &path, QString *errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(databaseText("无法读取数据文件：%1").arg(path), errorMessage);
        return {};
    }
    return splitLines(decodeGb18030(file.readAll()));
}

QString valueAt(const QMap<QString, QStringList> &columns, const QString &columnName, int row)
{
    return columns.value(columnName).value(row);
}

int maxRows(const QMap<QString, QStringList> &columns)
{
    int result = 0;
    for (const QStringList &values : columns)
        result = qMax(result, values.size());
    return result;
}

int toInt(const QString &value, int fallback = 0)
{
    bool ok = false;
    const int parsed = value.toInt(&ok);
    return ok ? parsed : fallback;
}

double toDouble(const QString &value, double fallback = 0.0)
{
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    return ok ? parsed : fallback;
}

QDateTime toDateTime(const QString &value)
{
    const QStringList formats = {
        QStringLiteral("yyyy/M/d HH:mm:ss"),
        QStringLiteral("yyyy/M/d H:mm:ss"),
        QStringLiteral("yyyy/M/d"),
    };
    for (const QString &format : formats) {
        const QDateTime dateTime = QDateTime::fromString(value, format);
        if (dateTime.isValid())
            return dateTime;
    }
    return {};
}

QString sqliteError(const QSqlDatabase &db)
{
    return db.lastError().text();
}

QString sqliteError(const QSqlDatabase &db, const QSqlQuery &query)
{
    const QString queryError = query.lastError().text();
    if (!queryError.isEmpty())
        return queryError;
    return sqliteError(db);
}

bool execSql(QSqlDatabase &db, const QString &sql, QString *errorMessage)
{
    QSqlQuery query(db);
    if (!query.exec(sql)) {
        setError(sqliteError(db, query), errorMessage);
        return false;
    }
    return true;
}

bool prepare(QSqlQuery &query, const QString &sql, QString *errorMessage)
{
    if (!query.prepare(sql)) {
        setError(query.lastError().text(), errorMessage);
        return false;
    }
    return true;
}

QString connectionName()
{
    static std::atomic<quint64> counter = 0;
    return QStringLiteral("gucds_sqlite_%1").arg(counter.fetch_add(1, std::memory_order_relaxed) + 1);
}

QSqlDatabase openSqlite(const QString &sqlitePath, const QString &name, QString *errorMessage)
{
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), name);
    db.setDatabaseName(sqlitePath);
    if (!db.open()) {
        setError(databaseText("无法打开 SQLite：%1").arg(db.lastError().text()), errorMessage);
    } else {
        QSqlQuery query(db);
        query.exec(QStringLiteral("PRAGMA busy_timeout=3000"));
        query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_device_records_identity_nocase ON device_records(category COLLATE NOCASE,name COLLATE NOCASE,model COLLATE NOCASE)"));
    }
    return db;
}

void bindDeviceRecord(QSqlQuery &query, const DeviceRecord &record)
{
    query.addBindValue(record.name.trimmed());
    query.addBindValue(record.category.trimmed());
    query.addBindValue(record.model.trimmed());
    query.addBindValue(record.data1.trimmed());
    query.addBindValue(record.data2.trimmed());
    query.addBindValue(record.data3.trimmed());
    query.addBindValue(record.data4.trimmed());
    query.addBindValue(record.data5.trimmed());
    query.addBindValue(record.modbus.trimmed());
    query.addBindValue(record.lora.trimmed());
    query.addBindValue(record.dtu.trimmed());
    query.addBindValue(record.parameter1.trimmed());
    query.addBindValue(record.parameter2.trimmed());
    query.addBindValue(record.parameter3.trimmed());
    query.addBindValue(record.parameter4.trimmed());
    query.addBindValue(record.parameter5.trimmed());
    query.addBindValue(record.calibrationPoints);
}

QVector<TdmsColumn> readTdmsColumns(const QString &path, QString *errorMessage)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        setError(databaseText("无法读取 TDMS：%1").arg(path), errorMessage);
        return {};
    }

    const QByteArray bytes = file.readAll();
    if (bytes.size() < 28 || bytes.left(4) != QByteArrayLiteral("TDSm")) {
        setError(databaseText("不是可识别的 TDMS 文件：%1").arg(path), errorMessage);
        return {};
    }

    struct RawInfo
    {
        QString objectPath;
        quint32 dataType = 0;
        quint64 valueCount = 0;
        quint64 rawBytes = 0;
        int columnIndex = 0;
    };

    qsizetype offset = 4;
    readU32(bytes, offset);
    readU32(bytes, offset);
    readU64(bytes, offset);
    const quint64 rawOffset = readU64(bytes, offset);
    const qsizetype rawStart = 28 + qsizetype(rawOffset);
    const quint32 objectCount = readU32(bytes, offset);
    QVector<RawInfo> rawInfos;
    rawInfos.reserve(int(objectCount));

    for (quint32 i = 0; i < objectCount; ++i) {
        const QString objectPath = readTdmsString(bytes, offset);
        const quint32 indexLength = readU32(bytes, offset);
        RawInfo rawInfo;
        rawInfo.objectPath = objectPath;
        bool hasRawData = false;
        if (indexLength != 0xFFFFFFFFu && indexLength != 0u) {
            const qsizetype indexEnd = offset + qsizetype(indexLength) - 4;
            rawInfo.dataType = readU32(bytes, offset);
            readU32(bytes, offset);
            rawInfo.valueCount = readU64(bytes, offset);
            if (offset + 8 <= indexEnd)
                rawInfo.rawBytes = readU64(bytes, offset);
            offset = indexEnd;
            hasRawData = rawInfo.dataType == 0x20;
        }

        const quint32 propertyCount = readU32(bytes, offset);
        for (quint32 property = 0; property < propertyCount; ++property) {
            const QString name = readTdmsString(bytes, offset);
            const quint32 type = readU32(bytes, offset);
            if (type == 0x20) {
                readTdmsString(bytes, offset);
            } else if (type == 0x03) {
                const quint32 value = readU32(bytes, offset);
                if (name == QStringLiteral("NI_ArrayColumn"))
                    rawInfo.columnIndex = int(value);
            } else if (type == 0x0A) {
                offset += 8;
            }
        }

        if (hasRawData)
            rawInfos.append(rawInfo);
    }

    QVector<TdmsColumn> columns;
    columns.reserve(rawInfos.size());
    qsizetype rawPosition = rawStart;
    const QRegularExpression pathPattern(QStringLiteral("^/'([^']*)'/'([^']*)'$"));
    for (const RawInfo &rawInfo : rawInfos) {
        if (rawPosition + qsizetype(rawInfo.rawBytes) > bytes.size()) {
            setError(databaseText("TDMS 原始数据越界：%1").arg(path), errorMessage);
            return {};
        }

        const QRegularExpressionMatch match = pathPattern.match(rawInfo.objectPath);
        rawPosition += qsizetype(rawInfo.rawBytes);
        if (!match.hasMatch())
            continue;

        const QByteArray raw = bytes.mid(rawPosition - qsizetype(rawInfo.rawBytes), qsizetype(rawInfo.rawBytes));
        const int count = int(rawInfo.valueCount);
        if (raw.size() < count * 4)
            continue;

        QVector<quint32> ends;
        ends.reserve(count);
        qsizetype offsetInRaw = 0;
        for (int i = 0; i < count; ++i)
            ends.append(readU32(raw, offsetInRaw));

        const QByteArray payload = raw.mid(count * 4);
        QStringList values;
        values.reserve(count);
        qsizetype previous = 0;
        for (quint32 end : ends) {
            const qsizetype safeEnd = qMin(qsizetype(end), payload.size());
            values.append(QString::fromUtf8(payload.constData() + previous, int(safeEnd - previous)));
            previous = safeEnd;
        }

        columns.append({match.captured(1), match.captured(2), rawInfo.columnIndex, values});
    }

    return columns;
}

TdmsGroups groupTdmsColumns(const QVector<TdmsColumn> &columns)
{
    TdmsGroups groups;
    for (const TdmsColumn &column : columns)
        groups[column.groupName].insert(column.columnName, column.values);
    return groups;
}

bool createSchema(QSqlDatabase &db, QString *errorMessage)
{
    const QStringList statements = {
        QStringLiteral("DROP TABLE IF EXISTS import_info"),
        QStringLiteral("DROP TABLE IF EXISTS device_records"),
        QStringLiteral("DROP TABLE IF EXISTS calibration_records"),
        QStringLiteral("DROP TABLE IF EXISTS bus_device_records"),
        QStringLiteral("DROP TABLE IF EXISTS device_categories"),
        QStringLiteral("DROP TABLE IF EXISTS frequency_tension_parameters"),
        QStringLiteral("DROP TABLE IF EXISTS labview_table_cells"),
        QStringLiteral("DROP TABLE IF EXISTS tdms_cells"),
        QStringLiteral("CREATE TABLE import_info(key TEXT PRIMARY KEY, value TEXT NOT NULL)"),
        QStringLiteral("CREATE TABLE device_records(id INTEGER PRIMARY KEY AUTOINCREMENT, source TEXT, group_name TEXT, row_order INTEGER, name TEXT, category TEXT, model TEXT, data1 TEXT, data2 TEXT, data3 TEXT, data4 TEXT, data5 TEXT, modbus TEXT, lora TEXT, dtu TEXT, parameter1 TEXT, parameter2 TEXT, parameter3 TEXT, parameter4 TEXT, parameter5 TEXT, calibration_points INTEGER)"),
        QStringLiteral("CREATE TABLE calibration_records(id INTEGER PRIMARY KEY AUTOINCREMENT, source TEXT, curve_name TEXT, row_order INTEGER, point INTEGER, measured_value REAL, reference_value REAL, temperature REAL, timestamp TEXT)"),
        QStringLiteral("CREATE TABLE bus_device_records(id INTEGER PRIMARY KEY AUTOINCREMENT, source TEXT, group_name TEXT, row_order INTEGER, device_index INTEGER, sensor_name TEXT, model TEXT, channel INTEGER, group_no INTEGER, address INTEGER, data_count INTEGER, response_code TEXT)"),
        QStringLiteral("CREATE TABLE device_categories(id INTEGER PRIMARY KEY AUTOINCREMENT, source TEXT, row_order INTEGER, name TEXT)"),
        QStringLiteral("CREATE TABLE frequency_tension_parameters(id INTEGER PRIMARY KEY AUTOINCREMENT, source TEXT, sensor_name TEXT, support_factor REAL, unit_mass REAL, cable_length REAL, area REAL, elastic_modulus REAL, inertia REAL, angle REAL)"),
        QStringLiteral("CREATE TABLE labview_table_cells(id INTEGER PRIMARY KEY AUTOINCREMENT, source TEXT, row_order INTEGER, column_order INTEGER, value TEXT)"),
        QStringLiteral("CREATE TABLE tdms_cells(id INTEGER PRIMARY KEY AUTOINCREMENT, source TEXT, group_name TEXT, column_name TEXT, row_order INTEGER, value TEXT)"),
        QStringLiteral("CREATE INDEX idx_device_records_identity ON device_records(category,name,model)"),
        QStringLiteral("CREATE INDEX idx_device_records_identity_nocase ON device_records(category COLLATE NOCASE,name COLLATE NOCASE,model COLLATE NOCASE)"),
    };

    for (const QString &statement : statements) {
        if (!execSql(db, statement, errorMessage))
            return false;
    }
    return true;
}

bool importTableCells(QSqlDatabase &db, const QString &sourceName, const QStringList &lines, QString *errorMessage)
{
    QSqlQuery query(db);
    if (!prepare(query, QStringLiteral("INSERT INTO labview_table_cells(source,row_order,column_order,value) VALUES(?,?,?,?)"), errorMessage))
        return false;
    for (int row = 0; row < lines.size(); ++row) {
        const QStringList cells = lines.at(row).split(QLatin1Char('\t'));
        for (int column = 0; column < cells.size(); ++column) {
            query.addBindValue(sourceName);
            query.addBindValue(row + 1);
            query.addBindValue(column + 1);
            query.addBindValue(cells.at(column));
            if (!query.exec()) {
                setError(query.lastError().text(), errorMessage);
                return false;
            }
        }
    }
    return true;
}

bool importNameTable(QSqlDatabase &db, const QStringList &lines, QString *errorMessage)
{
    QSqlQuery query(db);
    if (!prepare(query, QStringLiteral("INSERT INTO device_records(source,group_name,row_order,name,category,model,data1,data2,data3,data4,data5) VALUES('table/名称','快捷表',?,?,?,?,?,?,?,?,?)"), errorMessage))
        return false;

    for (int row = 0; row < lines.size(); ++row) {
        const QStringList cells = lines.at(row).split(QLatin1Char('\t'));
        query.addBindValue(row + 1);
        query.addBindValue(cells.value(0));
        query.addBindValue(cells.value(1));
        query.addBindValue(cells.value(2));
        query.addBindValue(cells.value(3));
        query.addBindValue(cells.value(4));
        query.addBindValue(cells.value(5));
        query.addBindValue(cells.value(6));
        query.addBindValue(QString());
        if (!query.exec()) {
            setError(query.lastError().text(), errorMessage);
            return false;
        }
    }
    return true;
}

bool importDeviceCategories(QSqlDatabase &db, const QString &path, QString *errorMessage)
{
    const QStringList lines = readGbTable(path, errorMessage);
    if (lines.isEmpty())
        return errorMessage == nullptr || errorMessage->isEmpty();

    QSqlQuery query(db);
    if (!prepare(query, QStringLiteral("INSERT INTO device_categories(source,row_order,name) VALUES('files/设备类别',?,?)"), errorMessage))
        return false;

    int order = 1;
    for (const QString &line : lines) {
        for (const QString &category : line.split(QLatin1Char('\t'), Qt::SkipEmptyParts)) {
            query.addBindValue(order++);
            query.addBindValue(category);
            if (!query.exec()) {
                setError(query.lastError().text(), errorMessage);
                return false;
            }
        }
    }
    return importTableCells(db, QStringLiteral("files/设备类别"), lines, errorMessage);
}

bool importFrequencyParameters(QSqlDatabase &db, const QString &path, QString *errorMessage)
{
    const QStringList lines = readGbTable(path, errorMessage);
    if (lines.isEmpty())
        return errorMessage == nullptr || errorMessage->isEmpty();

    QSqlQuery query(db);
    if (!prepare(query, QStringLiteral("INSERT INTO frequency_tension_parameters(source,sensor_name,support_factor,unit_mass,cable_length,area,elastic_modulus,inertia,angle) VALUES('files/频振索力传感器参数',?,?,?,?,?,?,?,?)"), errorMessage))
        return false;

    for (const QString &line : lines) {
        const QStringList cells = line.split(QLatin1Char('\t'));
        if (cells.isEmpty())
            continue;
        query.addBindValue(cells.value(0));
        query.addBindValue(toDouble(cells.value(1)));
        query.addBindValue(toDouble(cells.value(2)));
        query.addBindValue(toDouble(cells.value(3)));
        query.addBindValue(toDouble(cells.value(4)));
        query.addBindValue(toDouble(cells.value(5)));
        query.addBindValue(toDouble(cells.value(6)));
        query.addBindValue(toDouble(cells.value(7)));
        if (!query.exec()) {
            setError(query.lastError().text(), errorMessage);
            return false;
        }
    }
    return importTableCells(db, QStringLiteral("files/频振索力传感器参数"), lines, errorMessage);
}

bool importTdmsCells(QSqlDatabase &db, const QString &sourceName, const TdmsGroups &groups, QString *errorMessage)
{
    QSqlQuery query(db);
    if (!prepare(query, QStringLiteral("INSERT INTO tdms_cells(source,group_name,column_name,row_order,value) VALUES(?,?,?,?,?)"), errorMessage))
        return false;
    for (auto groupIt = groups.cbegin(); groupIt != groups.cend(); ++groupIt) {
        for (auto columnIt = groupIt.value().cbegin(); columnIt != groupIt.value().cend(); ++columnIt) {
            const QStringList values = columnIt.value();
            for (int row = 0; row < values.size(); ++row) {
                query.addBindValue(sourceName);
                query.addBindValue(groupIt.key());
                query.addBindValue(columnIt.key());
                query.addBindValue(row + 1);
                query.addBindValue(values.at(row));
                if (!query.exec()) {
                    setError(query.lastError().text(), errorMessage);
                    return false;
                }
            }
        }
    }
    return true;
}

bool importDeviceLibraryTdms(QSqlDatabase &db, const TdmsGroups &groups, QString *errorMessage)
{
    QSqlQuery query(db);
    if (!prepare(query, QStringLiteral("INSERT INTO device_records(source,group_name,row_order,name,category,model,data1,data2,data3,data4,data5,modbus,lora,dtu,parameter1,parameter2,parameter3,parameter4,parameter5,calibration_points) VALUES('tdms/设备库',?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"), errorMessage))
        return false;

    for (auto groupIt = groups.cbegin(); groupIt != groups.cend(); ++groupIt) {
        const QMap<QString, QStringList> columns = groupIt.value();
        const int rows = maxRows(columns);
        for (int row = 0; row < rows; ++row) {
            const QString productName = valueAt(columns, QStringLiteral("产品名称"), row);
            query.addBindValue(groupIt.key());
            query.addBindValue(row + 1);
            query.addBindValue(productName);
            query.addBindValue(valueAt(columns, QStringLiteral("产品类别"), row).isEmpty() ? groupIt.key() : valueAt(columns, QStringLiteral("产品类别"), row));
            query.addBindValue(productName);
            query.addBindValue(valueAt(columns, QStringLiteral("数据名1"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("数据名2"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("数据名3"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("数据名4"), row));
            query.addBindValue(QString());
            query.addBindValue(valueAt(columns, QStringLiteral("Modbus"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("LoRa"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("DTU"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("参数1"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("参数2"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("参数3"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("参数4"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("参数5"), row));
            query.addBindValue(toInt(valueAt(columns, QStringLiteral("标定点数"), row)));
            if (!query.exec()) {
                setError(query.lastError().text(), errorMessage);
                return false;
            }
        }
    }
    return true;
}

bool importCalibrationTdms(QSqlDatabase &db, const TdmsGroups &groups, QString *errorMessage)
{
    QSqlQuery query(db);
    if (!prepare(query, QStringLiteral("INSERT INTO calibration_records(source,curve_name,row_order,point,measured_value,reference_value,temperature,timestamp) VALUES('tdms/标定曲线',?,?,?,?,?,?,?)"), errorMessage))
        return false;

    for (auto groupIt = groups.cbegin(); groupIt != groups.cend(); ++groupIt) {
        const QMap<QString, QStringList> columns = groupIt.value();
        const int rows = maxRows(columns);
        for (int row = 0; row < rows; ++row) {
            query.addBindValue(groupIt.key());
            query.addBindValue(row + 1);
            query.addBindValue(toInt(valueAt(columns, QStringLiteral("点号"), row), row + 1));
            query.addBindValue(toDouble(valueAt(columns, QStringLiteral("测量值"), row)));
            query.addBindValue(toDouble(valueAt(columns, QStringLiteral("标定值"), row)));
            query.addBindValue(toDouble(valueAt(columns, QStringLiteral("温度(℃)"), row)));
            query.addBindValue(valueAt(columns, QStringLiteral("日期时间"), row));
            if (!query.exec()) {
                setError(query.lastError().text(), errorMessage);
                return false;
            }
        }
    }
    return true;
}

bool importBusTdms(QSqlDatabase &db, const TdmsGroups &groups, QString *errorMessage)
{
    QSqlQuery query(db);
    if (!prepare(query, QStringLiteral("INSERT INTO bus_device_records(source,group_name,row_order,device_index,sensor_name,model,channel,group_no,address,data_count,response_code) VALUES('tdms/总线设备管理器',?,?,?,?,?,?,?,?,?,?)"), errorMessage))
        return false;

    for (auto groupIt = groups.cbegin(); groupIt != groups.cend(); ++groupIt) {
        const QMap<QString, QStringList> columns = groupIt.value();
        const int rows = maxRows(columns);
        for (int row = 0; row < rows; ++row) {
            query.addBindValue(groupIt.key());
            query.addBindValue(row + 1);
            query.addBindValue(toInt(valueAt(columns, QStringLiteral("序号"), row), row + 1));
            query.addBindValue(valueAt(columns, QStringLiteral("传感器名"), row));
            query.addBindValue(valueAt(columns, QStringLiteral("规格型号"), row));
            query.addBindValue(toInt(valueAt(columns, QStringLiteral("信道"), row)));
            query.addBindValue(toInt(valueAt(columns, QStringLiteral("组号"), row)));
            query.addBindValue(toInt(valueAt(columns, QStringLiteral("地址"), row)));
            query.addBindValue(toInt(valueAt(columns, QStringLiteral("数据数"), row)));
            query.addBindValue(QString());
            if (!query.exec()) {
                setError(query.lastError().text(), errorMessage);
                return false;
            }
        }
    }
    return true;
}

} // namespace

QString LabviewDatabase::defaultDatabasePath()
{
    const QString envPath = qEnvironmentVariable("QLIOT_SQLITE_PATH");
    if (!envPath.isEmpty())
        return envPath;

    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList databaseCandidates = {
        QDir::current().filePath(QStringLiteral("data/gucds.sqlite")),
        QDir(appDir).filePath(QStringLiteral("data/gucds.sqlite")),
        QDir(appDir).absoluteFilePath(QStringLiteral("../data/gucds.sqlite")),
        QDir(appDir).absoluteFilePath(QStringLiteral("../../data/gucds.sqlite")),
        QDir(appDir).absoluteFilePath(QStringLiteral("../../../data/gucds.sqlite")),
    };

    for (const QString &candidate : databaseCandidates)
        if (QFileInfo::exists(candidate))
            return QFileInfo(candidate).absoluteFilePath();

    if (QFileInfo::exists(QDir::current().filePath(QStringLiteral("CMakeLists.txt"))))
        return QDir::current().absoluteFilePath(QStringLiteral("data/gucds.sqlite"));

    return QDir(appDir).filePath(QStringLiteral("data/gucds.sqlite"));
}

bool LabviewDatabase::importFromLabviewProject(const QString &labviewRoot, const QString &sqlitePath, QString *errorMessage)
{
    const QDir root(labviewRoot);
    if (!root.exists()) {
        setError(databaseText("LabVIEW 数据目录不存在：%1").arg(labviewRoot), errorMessage);
        return false;
    }

    QFileInfo sqliteInfo(sqlitePath);
    QDir().mkpath(sqliteInfo.absolutePath());
    if (sqliteInfo.exists() && !QFile::remove(sqlitePath)) {
        setError(databaseText("无法覆盖 SQLite 文件：%1").arg(sqlitePath), errorMessage);
        return false;
    }

    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (!db.isOpen())
            return false;

        if (!createSchema(db, errorMessage))
            return false;
        if (!db.transaction()) {
            setError(sqliteError(db), errorMessage);
            return false;
        }

        const QDir tableDir(root.filePath(QStringLiteral("table")));
        const QStringList tableNames = {QStringLiteral("名称"), QStringLiteral("性能"), QStringLiteral("传参"), QStringLiteral("MCU"), QStringLiteral("LoRa"), QStringLiteral("DTU")};
        for (const QString &tableName : tableNames) {
            const QStringList lines = readGbTable(tableDir.filePath(tableName), errorMessage);
            if (!errorMessage || errorMessage->isEmpty()) {
                if (!importTableCells(db, QStringLiteral("table/%1").arg(tableName), lines, errorMessage))
                    return false;
                if (tableName == QStringLiteral("名称") && !importNameTable(db, lines, errorMessage))
                    return false;
            } else {
                return false;
            }
        }

        const QDir filesDir(root.filePath(QStringLiteral("files")));
        if (!importDeviceCategories(db, filesDir.filePath(QStringLiteral("设备类别")), errorMessage))
            return false;
        if (!importFrequencyParameters(db, filesDir.filePath(QStringLiteral("频振索力传感器参数")), errorMessage))
            return false;

        const QVector<TdmsColumn> deviceColumns = readTdmsColumns(filesDir.filePath(QStringLiteral("设备库.tdms")), errorMessage);
        if (!errorMessage || errorMessage->isEmpty()) {
            const TdmsGroups groups = groupTdmsColumns(deviceColumns);
            if (!importTdmsCells(db, QStringLiteral("tdms/设备库"), groups, errorMessage) || !importDeviceLibraryTdms(db, groups, errorMessage))
                return false;
        } else {
            return false;
        }

        const QVector<TdmsColumn> calibrationColumns = readTdmsColumns(filesDir.filePath(QStringLiteral("标定曲线.tdms")), errorMessage);
        if (!errorMessage || errorMessage->isEmpty()) {
            const TdmsGroups groups = groupTdmsColumns(calibrationColumns);
            if (!importTdmsCells(db, QStringLiteral("tdms/标定曲线"), groups, errorMessage) || !importCalibrationTdms(db, groups, errorMessage))
                return false;
        } else {
            return false;
        }

        const QVector<TdmsColumn> busColumns = readTdmsColumns(filesDir.filePath(QStringLiteral("总线设备管理器.tdms")), errorMessage);
        if (!errorMessage || errorMessage->isEmpty()) {
            const TdmsGroups groups = groupTdmsColumns(busColumns);
            if (!importTdmsCells(db, QStringLiteral("tdms/总线设备管理器"), groups, errorMessage) || !importBusTdms(db, groups, errorMessage))
                return false;
        } else {
            return false;
        }

        QSqlQuery info(db);
        if (!prepare(info, QStringLiteral("INSERT INTO import_info(key,value) VALUES(?,?)"), errorMessage))
            return false;
        info.addBindValue(QStringLiteral("imported_at"));
        info.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
        if (!info.exec()) {
            setError(info.lastError().text(), errorMessage);
            return false;
        }
        info.addBindValue(QStringLiteral("labview_root"));
        info.addBindValue(QFileInfo(labviewRoot).absoluteFilePath());
        if (!info.exec()) {
            setError(info.lastError().text(), errorMessage);
            return false;
        }

        if (!db.commit()) {
            setError(sqliteError(db), errorMessage);
            return false;
        }
        db.close();
    }
    if (errorMessage)
        errorMessage->clear();
    return true;
}

QVector<DeviceRecord> LabviewDatabase::loadDeviceRecords(const QString &sqlitePath, QString *errorMessage)
{
    QVector<DeviceRecord> records;
    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    bool success = false;
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (db.isOpen()) {
            QSqlQuery query(db);
            if (!query.exec(QStringLiteral("SELECT id,name,category,model,data1,data2,data3,data4,data5,modbus,lora,dtu,parameter1,parameter2,parameter3,parameter4,parameter5,calibration_points FROM device_records ORDER BY CASE source WHEN 'tdms/设备库' THEN 0 ELSE 1 END, group_name, row_order, id"))) {
                setError(query.lastError().text(), errorMessage);
            } else {
                while (query.next()) {
                    DeviceRecord record;
                    record.databaseId = query.value(0).toLongLong();
                    record.name = query.value(1).toString();
                    record.category = query.value(2).toString();
                    record.model = query.value(3).toString();
                    record.data1 = query.value(4).toString();
                    record.data2 = query.value(5).toString();
                    record.data3 = query.value(6).toString();
                    record.data4 = query.value(7).toString();
                    record.data5 = query.value(8).toString();
                    record.modbus = query.value(9).toString();
                    record.lora = query.value(10).toString();
                    record.dtu = query.value(11).toString();
                    record.parameter1 = query.value(12).toString();
                    record.parameter2 = query.value(13).toString();
                    record.parameter3 = query.value(14).toString();
                    record.parameter4 = query.value(15).toString();
                    record.parameter5 = query.value(16).toString();
                    record.calibrationPoints = query.value(17).toInt();
                    records.append(record);
                }
                success = true;
            }
            db.close();
        }
    }
    if (success && errorMessage)
        errorMessage->clear();
    return records;
}

bool LabviewDatabase::saveDeviceRecord(const QString &sqlitePath, DeviceRecord *record, QString *errorMessage)
{
    if (!record) {
        setError(databaseText("产品记录不能为空"), errorMessage);
        return false;
    }
    if (record->name.trimmed().isEmpty() || record->category.trimmed().isEmpty() || record->model.trimmed().isEmpty()) {
        setError(databaseText("产品名称、产品类别和规格型号不能为空"), errorMessage);
        return false;
    }

    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    bool success = false;
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (db.isOpen()) {
            if (!db.transaction()) {
                setError(sqliteError(db), errorMessage);
            } else {
                QSqlQuery duplicate(db);
                if (!prepare(duplicate,
                             QStringLiteral("SELECT id FROM device_records WHERE trim(name)=? COLLATE NOCASE AND trim(category)=? COLLATE NOCASE AND trim(model)=? COLLATE NOCASE AND id<>? LIMIT 1"),
                             errorMessage)) {
                    db.rollback();
                } else {
                    duplicate.addBindValue(record->name.trimmed());
                    duplicate.addBindValue(record->category.trimmed());
                    duplicate.addBindValue(record->model.trimmed());
                    duplicate.addBindValue(record->databaseId > 0 ? record->databaseId : -1);
                    if (!duplicate.exec()) {
                        setError(duplicate.lastError().text(), errorMessage);
                        db.rollback();
                    } else if (duplicate.next()) {
                        setError(databaseText("相同名称、类别和型号的产品已经存在"), errorMessage);
                        db.rollback();
                    } else {
                        QSqlQuery write(db);
                        bool prepared = false;
                        if (record->databaseId > 0) {
                            prepared = prepare(
                                write,
                                QStringLiteral("UPDATE device_records SET group_name=?,name=?,category=?,model=?,data1=?,data2=?,data3=?,data4=?,data5=?,modbus=?,lora=?,dtu=?,parameter1=?,parameter2=?,parameter3=?,parameter4=?,parameter5=?,calibration_points=? WHERE id=?"),
                                errorMessage);
                            if (prepared)
                                write.addBindValue(record->category.trimmed());
                        } else {
                            prepared = prepare(
                                write,
                                QStringLiteral("INSERT INTO device_records(source,group_name,row_order,name,category,model,data1,data2,data3,data4,data5,modbus,lora,dtu,parameter1,parameter2,parameter3,parameter4,parameter5,calibration_points) VALUES('user/产品管理',?,(SELECT COALESCE(MAX(row_order),-1)+1 FROM device_records WHERE group_name=?),?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"),
                                errorMessage);
                            if (prepared) {
                                write.addBindValue(record->category.trimmed());
                                write.addBindValue(record->category.trimmed());
                            }
                        }

                        if (!prepared) {
                            db.rollback();
                        } else {
                            bindDeviceRecord(write, *record);
                            if (record->databaseId > 0)
                                write.addBindValue(record->databaseId);

                            if (!write.exec()) {
                                setError(write.lastError().text(), errorMessage);
                                db.rollback();
                            } else if (record->databaseId > 0 && write.numRowsAffected() != 1) {
                                setError(databaseText("要修改的产品已不存在"), errorMessage);
                                db.rollback();
                            } else if (!db.commit()) {
                                setError(sqliteError(db), errorMessage);
                                db.rollback();
                            } else {
                                if (record->databaseId <= 0)
                                    record->databaseId = write.lastInsertId().toLongLong();
                                success = true;
                            }
                        }
                    }
                }
            }
            db.close();
        }
    }
    if (success && errorMessage)
        errorMessage->clear();
    return success;
}

bool LabviewDatabase::deleteDeviceRecord(const QString &sqlitePath, qint64 databaseId, QString *errorMessage)
{
    if (databaseId <= 0) {
        setError(databaseText("产品记录 ID 无效"), errorMessage);
        return false;
    }

    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    bool success = false;
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (db.isOpen()) {
            QSqlQuery query(db);
            if (prepare(query, QStringLiteral("DELETE FROM device_records WHERE id=?"), errorMessage)) {
                query.addBindValue(databaseId);
                if (!query.exec()) {
                    setError(query.lastError().text(), errorMessage);
                } else if (query.numRowsAffected() != 1) {
                    setError(databaseText("要删除的产品已不存在"), errorMessage);
                } else {
                    success = true;
                }
            }
            db.close();
        }
    }
    if (success && errorMessage)
        errorMessage->clear();
    return success;
}

QVector<FrequencyTensionParameterRecord> LabviewDatabase::loadFrequencyTensionParameters(
    const QString &sqlitePath,
    QString *errorMessage)
{
    QVector<FrequencyTensionParameterRecord> records;
    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    bool success = false;
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (db.isOpen()) {
            QSqlQuery query(db);
            if (!query.exec(QStringLiteral(
                    "SELECT id,sensor_name,support_factor,unit_mass,cable_length,area,elastic_modulus,inertia,angle "
                    "FROM frequency_tension_parameters ORDER BY id"))) {
                setError(query.lastError().text(), errorMessage);
            } else {
                while (query.next()) {
                    FrequencyTensionParameterRecord record;
                    record.databaseId = query.value(0).toLongLong();
                    record.sensorName = query.value(1).toString();
                    record.supportFactor = query.value(2).toDouble();
                    record.unitMass = query.value(3).toDouble();
                    record.cableLength = query.value(4).toDouble();
                    record.area = query.value(5).toDouble();
                    record.elasticModulus = query.value(6).toDouble();
                    record.inertia = query.value(7).toDouble();
                    record.angle = query.value(8).toDouble();
                    records.append(record);
                }
                success = true;
            }
            db.close();
        }
    }
    if (success && errorMessage)
        errorMessage->clear();
    return records;
}

bool LabviewDatabase::saveFrequencyTensionParameters(
    const QString &sqlitePath,
    QVector<FrequencyTensionParameterRecord> *records,
    QString *errorMessage)
{
    if (!records) {
        setError(databaseText("频振索力参数记录不能为空"), errorMessage);
        return false;
    }

    QSet<QString> sensorNames;
    for (const FrequencyTensionParameterRecord &record : std::as_const(*records)) {
        const QString sensorName = record.sensorName.trimmed();
        if (sensorName.isEmpty()) {
            setError(databaseText("传感器名称不能为空"), errorMessage);
            return false;
        }
        const QString normalizedName = sensorName.toCaseFolded();
        if (sensorNames.contains(normalizedName)) {
            setError(databaseText("传感器名称“%1”重复").arg(sensorName), errorMessage);
            return false;
        }
        sensorNames.insert(normalizedName);

        if (!std::isfinite(record.supportFactor) || !std::isfinite(record.unitMass)
            || !std::isfinite(record.cableLength) || !std::isfinite(record.area)
            || !std::isfinite(record.elasticModulus) || !std::isfinite(record.inertia)
            || !std::isfinite(record.angle)) {
            setError(databaseText("频振索力参数包含非有限数值"), errorMessage);
            return false;
        }
        if (record.supportFactor <= 0.0 || record.unitMass <= 0.0
            || record.cableLength <= 0.0 || record.area <= 0.0
            || record.elasticModulus <= 0.0 || record.inertia <= 0.0) {
            setError(databaseText("支座系数、单位质量、索长、截面积、弹性模量和截面惯性矩必须大于 0"),
                     errorMessage);
            return false;
        }
    }

    QVector<FrequencyTensionParameterRecord> savedRecords = *records;
    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    bool success = false;
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (db.isOpen()) {
            if (!db.transaction()) {
                setError(sqliteError(db), errorMessage);
            } else {
                bool writeSucceeded = true;
                for (FrequencyTensionParameterRecord &record : savedRecords) {
                    QSqlQuery write(db);
                    const QString sql = record.databaseId > 0
                        ? QStringLiteral(
                              "UPDATE frequency_tension_parameters SET sensor_name=?,support_factor=?,unit_mass=?,cable_length=?,area=?,elastic_modulus=?,inertia=?,angle=? WHERE id=?")
                        : QStringLiteral(
                              "INSERT INTO frequency_tension_parameters(source,sensor_name,support_factor,unit_mass,cable_length,area,elastic_modulus,inertia,angle) VALUES('user/频振索力传感器参数',?,?,?,?,?,?,?,?)");
                    if (!prepare(write, sql, errorMessage)) {
                        writeSucceeded = false;
                        break;
                    }
                    write.addBindValue(record.sensorName.trimmed());
                    write.addBindValue(record.supportFactor);
                    write.addBindValue(record.unitMass);
                    write.addBindValue(record.cableLength);
                    write.addBindValue(record.area);
                    write.addBindValue(record.elasticModulus);
                    write.addBindValue(record.inertia);
                    write.addBindValue(record.angle);
                    if (record.databaseId > 0)
                        write.addBindValue(record.databaseId);
                    if (!write.exec()) {
                        setError(write.lastError().text(), errorMessage);
                        writeSucceeded = false;
                        break;
                    }
                    if (record.databaseId <= 0) {
                        record.databaseId = write.lastInsertId().toLongLong();
                    } else if (write.numRowsAffected() != 1) {
                        QSqlQuery existing(db);
                        if (!prepare(existing,
                                     QStringLiteral("SELECT 1 FROM frequency_tension_parameters WHERE id=?"),
                                     errorMessage)) {
                            writeSucceeded = false;
                            break;
                        }
                        existing.addBindValue(record.databaseId);
                        if (!existing.exec()) {
                            setError(existing.lastError().text(), errorMessage);
                            writeSucceeded = false;
                            break;
                        }
                        if (!existing.next()) {
                            setError(databaseText("要修改的频振索力参数已不存在"), errorMessage);
                            writeSucceeded = false;
                            break;
                        }
                    }
                }

                if (!writeSucceeded) {
                    db.rollback();
                } else if (!db.commit()) {
                    setError(sqliteError(db), errorMessage);
                    db.rollback();
                } else {
                    *records = savedRecords;
                    success = true;
                }
            }
            db.close();
        }
    }
    if (success && errorMessage)
        errorMessage->clear();
    return success;
}

QVector<CalibrationRecord> LabviewDatabase::loadCalibrationRecords(const QString &sqlitePath, QString *errorMessage)
{
    QVector<CalibrationRecord> records;
    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (!db.isOpen())
            return records;
        QSqlQuery query(db);
        if (!query.exec(QStringLiteral("SELECT id,curve_name,point,measured_value,reference_value,temperature,timestamp FROM calibration_records ORDER BY curve_name,row_order,id"))) {
            setError(query.lastError().text(), errorMessage);
            return records;
        }
        while (query.next()) {
            CalibrationRecord record;
            record.databaseId = query.value(0).toLongLong();
            record.curveName = query.value(1).toString();
            record.point = query.value(2).toInt();
            record.measuredValue = query.value(3).toDouble();
            record.referenceValue = query.value(4).toDouble();
            record.temperature = query.value(5).toDouble();
            record.timestamp = toDateTime(query.value(6).toString());
            records.append(record);
        }
        db.close();
    }
    if (errorMessage)
        errorMessage->clear();
    return records;
}

bool LabviewDatabase::saveCalibrationRecord(const QString &sqlitePath,
                                            CalibrationRecord *record,
                                            QString *errorMessage)
{
    if (!record || record->curveName.trimmed().isEmpty() || record->point < 0) {
        setError(databaseText("标定记录的曲线名或点号无效"), errorMessage);
        return false;
    }
    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    bool success = false;
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (db.isOpen()) {
            QSqlQuery query(db);
            const QString sql = record->databaseId > 0
                ? QStringLiteral("UPDATE calibration_records SET curve_name=?,point=?,measured_value=?,reference_value=?,temperature=?,timestamp=? WHERE id=?")
                : QStringLiteral("INSERT INTO calibration_records(source,curve_name,row_order,point,measured_value,reference_value,temperature,timestamp) VALUES('user/标定',?,(SELECT COALESCE(MAX(row_order),-1)+1 FROM calibration_records),?,?,?,?,?)");
            if (prepare(query, sql, errorMessage)) {
                query.addBindValue(record->curveName.trimmed());
                query.addBindValue(record->point);
                query.addBindValue(record->measuredValue);
                query.addBindValue(record->referenceValue);
                query.addBindValue(record->temperature);
                query.addBindValue(record->timestamp.isValid()
                                       ? record->timestamp.toString(QStringLiteral("yyyy/M/d HH:mm:ss"))
                                       : QDateTime::currentDateTime().toString(QStringLiteral("yyyy/M/d HH:mm:ss")));
                if (record->databaseId > 0)
                    query.addBindValue(record->databaseId);
                if (!query.exec()) {
                    setError(query.lastError().text(), errorMessage);
                } else if (record->databaseId > 0 && query.numRowsAffected() != 1) {
                    setError(databaseText("要修改的标定记录已不存在"), errorMessage);
                } else {
                    if (record->databaseId <= 0)
                        record->databaseId = query.lastInsertId().toLongLong();
                    success = true;
                }
            }
            db.close();
        }
    }
    if (success && errorMessage)
        errorMessage->clear();
    return success;
}

bool LabviewDatabase::deleteCalibrationRecord(const QString &sqlitePath,
                                              qint64 databaseId,
                                              QString *errorMessage)
{
    if (databaseId <= 0) {
        setError(databaseText("标定记录 ID 无效"), errorMessage);
        return false;
    }
    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    bool success = false;
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (db.isOpen()) {
            QSqlQuery query(db);
            if (prepare(query, QStringLiteral("DELETE FROM calibration_records WHERE id=?"), errorMessage)) {
                query.addBindValue(databaseId);
                success = query.exec() && query.numRowsAffected() == 1;
                if (!success)
                    setError(query.lastError().text().isEmpty() ? databaseText("要删除的标定记录已不存在") : query.lastError().text(), errorMessage);
            }
            db.close();
        }
    }
    if (success && errorMessage)
        errorMessage->clear();
    return success;
}

QVector<BusDeviceRecord> LabviewDatabase::loadBusDeviceRecords(const QString &sqlitePath, QString *errorMessage)
{
    QVector<BusDeviceRecord> records;
    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (!db.isOpen())
            return records;
        QSqlQuery query(db);
        if (!query.exec(QStringLiteral("SELECT id,device_index,sensor_name,model,channel,group_no,address,data_count,response_code FROM bus_device_records ORDER BY row_order,id"))) {
            setError(query.lastError().text(), errorMessage);
            return records;
        }
        while (query.next()) {
            BusDeviceRecord record;
            record.databaseId = query.value(0).toLongLong();
            record.index = query.value(1).toInt();
            record.sensorName = query.value(2).toString();
            record.model = query.value(3).toString();
            record.channel = query.value(4).toInt();
            record.group = query.value(5).toInt();
            record.address = query.value(6).toInt();
            record.dataCount = query.value(7).toInt();
            record.responseCode = query.value(8).toString();
            records.append(record);
        }
        db.close();
    }
    if (errorMessage)
        errorMessage->clear();
    return records;
}

bool LabviewDatabase::saveBusDeviceRecord(const QString &sqlitePath,
                                          BusDeviceRecord *record,
                                          QString *errorMessage)
{
    if (!record || record->sensorName.trimmed().isEmpty() || record->address < 0 || record->address > 255) {
        setError(databaseText("总线设备名称或地址无效"), errorMessage);
        return false;
    }
    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    bool success = false;
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (db.isOpen()) {
            QSqlQuery query(db);
            const QString sql = record->databaseId > 0
                ? QStringLiteral("UPDATE bus_device_records SET device_index=?,sensor_name=?,model=?,channel=?,group_no=?,address=?,data_count=?,response_code=? WHERE id=?")
                : QStringLiteral("INSERT INTO bus_device_records(source,group_name,row_order,device_index,sensor_name,model,channel,group_no,address,data_count,response_code) VALUES('user/总线设备','用户设备',(SELECT COALESCE(MAX(row_order),-1)+1 FROM bus_device_records),?,?,?,?,?,?,?,?)");
            if (prepare(query, sql, errorMessage)) {
                query.addBindValue(record->index);
                query.addBindValue(record->sensorName.trimmed());
                query.addBindValue(record->model.trimmed());
                query.addBindValue(record->channel);
                query.addBindValue(record->group);
                query.addBindValue(record->address);
                query.addBindValue(record->dataCount);
                query.addBindValue(record->responseCode.trimmed());
                if (record->databaseId > 0)
                    query.addBindValue(record->databaseId);
                if (!query.exec()) {
                    setError(query.lastError().text(), errorMessage);
                } else if (record->databaseId > 0 && query.numRowsAffected() != 1) {
                    setError(databaseText("要修改的总线设备已不存在"), errorMessage);
                } else {
                    if (record->databaseId <= 0)
                        record->databaseId = query.lastInsertId().toLongLong();
                    success = true;
                }
            }
            db.close();
        }
    }
    if (success && errorMessage)
        errorMessage->clear();
    return success;
}

bool LabviewDatabase::deleteBusDeviceRecord(const QString &sqlitePath,
                                            qint64 databaseId,
                                            QString *errorMessage)
{
    if (databaseId <= 0) {
        setError(databaseText("总线设备记录 ID 无效"), errorMessage);
        return false;
    }
    const QString name = connectionName();
    const auto removeConnection = qScopeGuard([name] { QSqlDatabase::removeDatabase(name); });
    bool success = false;
    {
        QSqlDatabase db = openSqlite(sqlitePath, name, errorMessage);
        if (db.isOpen()) {
            QSqlQuery query(db);
            if (prepare(query, QStringLiteral("DELETE FROM bus_device_records WHERE id=?"), errorMessage)) {
                query.addBindValue(databaseId);
                success = query.exec() && query.numRowsAffected() == 1;
                if (!success)
                    setError(query.lastError().text().isEmpty() ? databaseText("要删除的总线设备已不存在") : query.lastError().text(), errorMessage);
            }
            db.close();
        }
    }
    if (success && errorMessage)
        errorMessage->clear();
    return success;
}

} // namespace gucds

#pragma once

#include "gucds/core/records.h"

#include <QString>
#include <QVector>

namespace gucds {

class LabviewDatabase
{
public:
    static QString defaultDatabasePath();
    static bool importFromLabviewProject(const QString &labviewRoot, const QString &sqlitePath, QString *errorMessage = nullptr);
    static QVector<DeviceRecord> loadDeviceRecords(const QString &sqlitePath, QString *errorMessage = nullptr);
    static bool saveDeviceRecord(const QString &sqlitePath, DeviceRecord *record, QString *errorMessage = nullptr);
    static bool deleteDeviceRecord(const QString &sqlitePath, qint64 databaseId, QString *errorMessage = nullptr);
    static QVector<FrequencyTensionParameterRecord> loadFrequencyTensionParameters(
        const QString &sqlitePath,
        QString *errorMessage = nullptr);
    static bool saveFrequencyTensionParameters(
        const QString &sqlitePath,
        QVector<FrequencyTensionParameterRecord> *records,
        QString *errorMessage = nullptr);
    static QVector<CalibrationRecord> loadCalibrationRecords(const QString &sqlitePath, QString *errorMessage = nullptr);
    static bool saveCalibrationRecord(const QString &sqlitePath, CalibrationRecord *record, QString *errorMessage = nullptr);
    static bool deleteCalibrationRecord(const QString &sqlitePath, qint64 databaseId, QString *errorMessage = nullptr);
    static QVector<BusDeviceRecord> loadBusDeviceRecords(const QString &sqlitePath, QString *errorMessage = nullptr);
    static bool saveBusDeviceRecord(const QString &sqlitePath, BusDeviceRecord *record, QString *errorMessage = nullptr);
    static bool deleteBusDeviceRecord(const QString &sqlitePath, qint64 databaseId, QString *errorMessage = nullptr);
};

} // namespace gucds

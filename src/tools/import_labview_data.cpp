#include "gucds/core/labviewdatabase.h"

#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    QTextStream err(stderr);

    const QStringList args = app.arguments();
    if (args.size() != 3) {
        err << "usage: " << args.value(0) << " <labview-root> <sqlite-path>\n";
        return 2;
    }

    QString errorMessage;
    if (!gucds::LabviewDatabase::importFromLabviewProject(args.at(1), args.at(2), &errorMessage)) {
        err << errorMessage << "\n";
        return 1;
    }

    out << "imported " << args.at(1) << " -> " << args.at(2) << "\n";

    const QVector<gucds::DeviceRecord> devices = gucds::LabviewDatabase::loadDeviceRecords(args.at(2), &errorMessage);
    if (!errorMessage.isEmpty()) {
        err << errorMessage << "\n";
        return 1;
    }
    const QVector<gucds::CalibrationRecord> calibrations = gucds::LabviewDatabase::loadCalibrationRecords(args.at(2), &errorMessage);
    if (!errorMessage.isEmpty()) {
        err << errorMessage << "\n";
        return 1;
    }
    const QVector<gucds::BusDeviceRecord> busDevices = gucds::LabviewDatabase::loadBusDeviceRecords(args.at(2), &errorMessage);
    if (!errorMessage.isEmpty()) {
        err << errorMessage << "\n";
        return 1;
    }

    out << "visible records: devices=" << devices.size()
        << ", calibrations=" << calibrations.size()
        << ", busDevices=" << busDevices.size() << "\n";
    return 0;
}

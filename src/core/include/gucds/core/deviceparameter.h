#pragma once

#include "gucds/core/records.h"

#include <QString>
#include <QStringList>

namespace gucds {

struct DeviceParameterDefinition
{
    QString name;
    QString editorMode;
    QString valueType;
    QString valueText;

    bool isValid() const;
    QStringList options() const;
};

DeviceParameterDefinition parseDeviceParameterDefinition(const QString &text);
QString formatDeviceParameterDefinition(const DeviceParameterDefinition &definition);
QString deviceParameterDefinition(const DeviceRecord &record, int index);
void setDeviceParameterDefinition(DeviceRecord *record, int index, const QString &value);
QString deviceParameterValue(const DeviceRecord &record, int index);
void setDeviceParameterValue(DeviceRecord *record, int index, const QString &value);

} // namespace gucds

#include "gucds/core/deviceparameter.h"

namespace gucds {

bool DeviceParameterDefinition::isValid() const
{
    return !name.trimmed().isEmpty() && name.trimmed() != QStringLiteral("未定义");
}

QStringList DeviceParameterDefinition::options() const
{
    QStringList result;
    for (const QString &option : valueText.split(QLatin1Char(','), Qt::SkipEmptyParts)) {
        const QString trimmed = option.trimmed();
        if (!trimmed.isEmpty())
            result.append(trimmed);
    }
    return result;
}

DeviceParameterDefinition parseDeviceParameterDefinition(const QString &text)
{
    DeviceParameterDefinition definition;
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty() || trimmed == QStringLiteral("未定义"))
        return definition;

    const int colonIndex = trimmed.indexOf(QLatin1Char(':'));
    const QString metadata = colonIndex >= 0 ? trimmed.left(colonIndex) : trimmed;
    const QStringList parts = metadata.split(QLatin1Char(','), Qt::KeepEmptyParts);
    definition.name = parts.value(0).trimmed();
    definition.editorMode = parts.value(1).trimmed();
    definition.valueType = parts.value(2).trimmed();
    definition.valueText = colonIndex >= 0 ? trimmed.mid(colonIndex + 1).trimmed() : QString();

    if (definition.editorMode.isEmpty())
        definition.editorMode = QStringLiteral("字符");
    if (definition.valueType.isEmpty())
        definition.valueType = QStringLiteral("字符");
    return definition;
}

QString formatDeviceParameterDefinition(const DeviceParameterDefinition &definition)
{
    if (!definition.isValid())
        return QStringLiteral("未定义");

    const QString editorMode = definition.editorMode.trimmed().isEmpty()
        ? QStringLiteral("字符")
        : definition.editorMode.trimmed();
    const QString valueType = definition.valueType.trimmed().isEmpty()
        ? QStringLiteral("字符")
        : definition.valueType.trimmed();
    return QStringLiteral("%1,%2,%3:%4")
        .arg(definition.name.trimmed(), editorMode, valueType, definition.valueText.trimmed());
}

QString deviceParameterDefinition(const DeviceRecord &record, int index)
{
    switch (index) {
    case 1:
        return record.parameter1;
    case 2:
        return record.parameter2;
    case 3:
        return record.parameter3;
    case 4:
        return record.parameter4;
    case 5:
        return record.parameter5;
    default:
        return {};
    }
}

void setDeviceParameterDefinition(DeviceRecord *record, int index, const QString &value)
{
    if (!record)
        return;

    switch (index) {
    case 1:
        record->parameter1 = value;
        break;
    case 2:
        record->parameter2 = value;
        break;
    case 3:
        record->parameter3 = value;
        break;
    case 4:
        record->parameter4 = value;
        break;
    case 5:
        record->parameter5 = value;
        break;
    default:
        break;
    }
}

QString deviceParameterValue(const DeviceRecord &record, int index)
{
    switch (index) {
    case 1:
        return record.parameterValue1;
    case 2:
        return record.parameterValue2;
    case 3:
        return record.parameterValue3;
    case 4:
        return record.parameterValue4;
    case 5:
        return record.parameterValue5;
    default:
        return {};
    }
}

void setDeviceParameterValue(DeviceRecord *record, int index, const QString &value)
{
    if (!record)
        return;

    switch (index) {
    case 1:
        record->parameterValue1 = value;
        break;
    case 2:
        record->parameterValue2 = value;
        break;
    case 3:
        record->parameterValue3 = value;
        break;
    case 4:
        record->parameterValue4 = value;
        break;
    case 5:
        record->parameterValue5 = value;
        break;
    default:
        break;
    }
}

} // namespace gucds

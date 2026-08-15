#include "gucds/core/delimitedtable.h"

#include <QFile>
#include <QRegularExpression>
#include <QStringConverter>
#include <QStringDecoder>
#include <QStringEncoder>
#include <QTextStream>

namespace gucds {

DelimitedTable::Rows DelimitedTable::read(const QString &filePath,
                                          QString *errorMessage,
                                          const QByteArray &encoding,
                                          QChar delimiter)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage)
            *errorMessage = file.errorString();
        return {};
    }

    const QByteArray bytes = file.readAll();
    QStringDecoder decoder(encoding.constData());
    const QString text = decoder.isValid()
        ? decoder.decode(bytes)
        : QString::fromLocal8Bit(bytes);

    Rows rows;
    const auto lines = text.split(QRegularExpression(QStringLiteral("\\r?\\n")), Qt::SkipEmptyParts);
    rows.reserve(lines.size());
    for (const QString &line : lines)
        rows.append(line.split(delimiter));
    return rows;
}

bool DelimitedTable::write(const QString &filePath,
                           const Rows &rows,
                           QString *errorMessage,
                           const QByteArray &encoding,
                           QChar delimiter)
{
    QString text;
    for (const QStringList &row : rows) {
        text += row.join(delimiter);
        text += QLatin1Char('\n');
    }

    QStringEncoder encoder(encoding.constData());
    const QByteArray bytes = encoder.isValid()
        ? encoder.encode(text)
        : text.toUtf8();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    if (file.write(bytes) != bytes.size()) {
        if (errorMessage)
            *errorMessage = file.errorString();
        return false;
    }

    return true;
}

} // namespace gucds

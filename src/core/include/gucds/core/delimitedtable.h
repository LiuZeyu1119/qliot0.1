#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace gucds {

class DelimitedTable
{
public:
    using Rows = QVector<QStringList>;

    static Rows read(const QString &filePath,
                     QString *errorMessage = nullptr,
                     const QByteArray &encoding = QByteArrayLiteral("GB18030"),
                     QChar delimiter = QLatin1Char('\t'));

    static bool write(const QString &filePath,
                      const Rows &rows,
                      QString *errorMessage = nullptr,
                      const QByteArray &encoding = QByteArrayLiteral("UTF-8"),
                      QChar delimiter = QLatin1Char('\t'));
};

} // namespace gucds

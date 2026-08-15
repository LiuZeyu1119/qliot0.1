#pragma once

#include <QHash>
#include <QString>
#include <QTranslator>

#include <memory>

class TsTranslator final : public QTranslator
{
public:
    using QTranslator::QTranslator;

    bool loadTs(const QString &path);
    void clear();
    bool isEmpty() const override;
    QString translate(const char *context,
                      const char *sourceText,
                      const char *disambiguation = nullptr,
                      int n = -1) const override;

private:
    static QString key(const QString &context, const QString &sourceText, const QString &comment = {});

    QHash<QString, QString> m_translations;
};

class ApplicationTranslator
{
public:
    ~ApplicationTranslator();

    static QString chineseLanguage();
    static QString englishLanguage();
    static QString currentLanguage();

    bool applyStoredLanguage();
    bool setLanguage(const QString &language);

private:
    static QString normalizedLanguage(const QString &language);

    TsTranslator m_applicationTranslator;
    std::unique_ptr<QTranslator> m_qtTranslator = std::make_unique<QTranslator>();
};

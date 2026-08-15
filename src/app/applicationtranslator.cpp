#include "applicationtranslator.h"

#include <QApplication>
#include <QFile>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QXmlStreamReader>

namespace {

constexpr auto kLanguageSetting = "ui/language";
constexpr auto kLanguageProperty = "uiLanguage";

} // namespace

bool TsTranslator::loadTs(const QString &path)
{
    m_translations.clear();
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QXmlStreamReader xml(&file);
    QString contextName;
    while (xml.readNextStartElement()) {
        if (xml.name() != QLatin1String("TS")) {
            xml.skipCurrentElement();
            continue;
        }

        while (xml.readNextStartElement()) {
            if (xml.name() != QLatin1String("context")) {
                xml.skipCurrentElement();
                continue;
            }

            contextName.clear();
            while (xml.readNextStartElement()) {
                if (xml.name() == QLatin1String("name")) {
                    contextName = xml.readElementText();
                    continue;
                }
                if (xml.name() != QLatin1String("message")) {
                    xml.skipCurrentElement();
                    continue;
                }

                QString source;
                QString translation;
                QString comment;
                bool unfinished = false;
                while (xml.readNextStartElement()) {
                    if (xml.name() == QLatin1String("source")) {
                        source = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    } else if (xml.name() == QLatin1String("translation")) {
                        unfinished = xml.attributes().value(QLatin1String("type")) == QLatin1String("unfinished");
                        translation = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    } else if (xml.name() == QLatin1String("comment")) {
                        comment = xml.readElementText(QXmlStreamReader::IncludeChildElements);
                    } else {
                        xml.skipCurrentElement();
                    }
                }
                if (!contextName.isEmpty() && !source.isEmpty() && !translation.isEmpty() && !unfinished)
                    m_translations.insert(key(contextName, source, comment), translation);
            }
        }
    }
    return !xml.hasError() && !m_translations.isEmpty();
}

bool TsTranslator::isEmpty() const
{
    return m_translations.isEmpty();
}

void TsTranslator::clear()
{
    m_translations.clear();
}

QString TsTranslator::translate(const char *context,
                                const char *sourceText,
                                const char *disambiguation,
                                int n) const
{
    Q_UNUSED(n)
    const QString contextString = QString::fromUtf8(context ? context : "");
    const QString sourceString = QString::fromUtf8(sourceText ? sourceText : "");
    const QString commentString = QString::fromUtf8(disambiguation ? disambiguation : "");
    QString translated = m_translations.value(key(contextString, sourceString, commentString));
    if (translated.isNull() && !commentString.isEmpty())
        translated = m_translations.value(key(contextString, sourceString));
    return translated;
}

QString TsTranslator::key(const QString &context, const QString &sourceText, const QString &comment)
{
    return context + QChar(0x1f) + sourceText + QChar(0x1f) + comment;
}

QString ApplicationTranslator::chineseLanguage()
{
    return QStringLiteral("zh_CN");
}

ApplicationTranslator::~ApplicationTranslator()
{
    if (!qApp)
        return;
    qApp->removeTranslator(&m_applicationTranslator);
    qApp->removeTranslator(m_qtTranslator.get());
}

QString ApplicationTranslator::englishLanguage()
{
    return QStringLiteral("en_US");
}

QString ApplicationTranslator::currentLanguage()
{
    const QString language = qApp->property(kLanguageProperty).toString();
    return language.isEmpty() ? chineseLanguage() : language;
}

bool ApplicationTranslator::applyStoredLanguage()
{
    QSettings settings;
    const QString configuredLanguage = settings.value(QLatin1String(kLanguageSetting)).toString();
    const QString initialLanguage = configuredLanguage.isEmpty()
        ? normalizedLanguage(QLocale::system().name())
        : normalizedLanguage(configuredLanguage);
    return setLanguage(initialLanguage);
}

bool ApplicationTranslator::setLanguage(const QString &language)
{
    const QString normalized = normalizedLanguage(language);
    qApp->removeTranslator(&m_applicationTranslator);
    qApp->removeTranslator(m_qtTranslator.get());
    m_applicationTranslator.clear();
    m_qtTranslator = std::make_unique<QTranslator>();

    if (normalized == englishLanguage()
        && !m_applicationTranslator.loadTs(QStringLiteral(":/i18n/translations/gucds_en_US.ts"))) {
        return false;
    }

    if (normalized == chineseLanguage()) {
        bool qtTranslationLoaded = m_qtTranslator->load(
            QLocale(normalized),
            QStringLiteral("qt"),
            QStringLiteral("_"),
            QLibraryInfo::path(QLibraryInfo::TranslationsPath));
        if (!qtTranslationLoaded) {
            qtTranslationLoaded = m_qtTranslator->load(
                QLocale(normalized),
                QStringLiteral("qtbase"),
                QStringLiteral("_"),
                QLibraryInfo::path(QLibraryInfo::TranslationsPath));
        }
        Q_UNUSED(qtTranslationLoaded)
    }

    if (!m_qtTranslator->isEmpty())
        qApp->installTranslator(m_qtTranslator.get());
    if (!m_applicationTranslator.isEmpty())
        qApp->installTranslator(&m_applicationTranslator);

    QLocale::setDefault(QLocale(normalized));
    qApp->setProperty(kLanguageProperty, normalized);
    QSettings().setValue(QLatin1String(kLanguageSetting), normalized);
    return true;
}

QString ApplicationTranslator::normalizedLanguage(const QString &language)
{
    return language.startsWith(QStringLiteral("en"), Qt::CaseInsensitive)
        ? englishLanguage()
        : chineseLanguage();
}

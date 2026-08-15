#include "applicationtranslator.h"
#include "mainwindow.h"

#include "gucds/core/appconfig.h"

#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(gucds::AppConfig::organizationName());
    QCoreApplication::setApplicationName(gucds::AppConfig::applicationTitle());
    QCoreApplication::setApplicationVersion(QStringLiteral(GUCDS_APP_VERSION));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/icon.png")));

    ApplicationTranslator translator;
    if (!translator.applyStoredLanguage())
        translator.setLanguage(ApplicationTranslator::chineseLanguage());

    MainWindow window;
    QObject::connect(&window,
                     &MainWindow::languageChangeRequested,
                     &app,
                     [&](const QString &language) {
                         if (language == ApplicationTranslator::currentLanguage())
                             return;
                         if (!translator.setLanguage(language)) {
                             QMessageBox::critical(&window,
                                                   QCoreApplication::translate("main", "语言切换失败"),
                                                   QCoreApplication::translate("main", "无法加载所选语言资源。"));
                             return;
                         }
                         QTimer::singleShot(0, &app, &QCoreApplication::quit);
                     });
    window.showMaximized();

    return app.exec();
}

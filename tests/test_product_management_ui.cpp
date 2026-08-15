#include "gucds/widgets/productmanagementdialog.h"
#include "gucds/core/labviewdatabase.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QTableWidget>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class ProductManagementUiTest : public QObject
{
    Q_OBJECT

private slots:
    void productWorkflowIsUsable();
};

void ProductManagementUiTest::productWorkflowIsUsable()
{
    const QString sourceDatabase = gucds::LabviewDatabase::defaultDatabasePath();
    QVERIFY2(QFile::exists(sourceDatabase), qPrintable(QStringLiteral("Product database was not found: %1").arg(sourceDatabase)));
    QTemporaryDir temporaryDirectory;
    QVERIFY(temporaryDirectory.isValid());
    const QString databasePath = temporaryDirectory.filePath(QStringLiteral("gucds.sqlite"));
    QVERIFY(QFile::copy(sourceDatabase, databasePath));

    QElapsedTimer loadTimer;
    loadTimer.start();
    ProductManagementDialog dialog(databasePath);
    dialog.show();
    QCoreApplication::processEvents();
    QVERIFY2(loadTimer.elapsed() < 3000, "Product management dialog took more than 3 seconds to load");

    auto *table = dialog.findChild<QTableView *>(QStringLiteral("productTable"));
    auto *searchEdit = dialog.findChild<QLineEdit *>(QStringLiteral("productSearchEdit"));
    auto *categoryFilter = dialog.findChild<QComboBox *>(QStringLiteral("productCategoryFilter"));
    auto *nameEdit = dialog.findChild<QLineEdit *>(QStringLiteral("productNameEdit"));
    auto *categoryEdit = dialog.findChild<QComboBox *>(QStringLiteral("productCategoryEdit"));
    auto *modelEdit = dialog.findChild<QLineEdit *>(QStringLiteral("productModelEdit"));
    auto *parameterTable = dialog.findChild<QTableWidget *>(QStringLiteral("productParameterTable"));
    auto *newButton = dialog.findChild<QPushButton *>(QStringLiteral("newProductButton"));
    auto *saveButton = dialog.findChild<QPushButton *>(QStringLiteral("saveProductButton"));
    QVERIFY(table);
    QVERIFY(searchEdit);
    QVERIFY(categoryFilter);
    QVERIFY(nameEdit);
    QVERIFY(categoryEdit);
    QVERIFY(modelEdit);
    QVERIFY(parameterTable);
    QVERIFY(newButton);
    QVERIFY(saveButton);

    const int originalCount = table->model()->rowCount();
    QVERIFY(originalCount >= 140);
    QCOMPARE(parameterTable->rowCount(), 5);
    QCOMPARE(parameterTable->columnCount(), 4);
    QVERIFY(!nameEdit->text().isEmpty());

    searchEdit->setText(QStringLiteral("QL-SPS-WNDUG-1"));
    QTRY_VERIFY_WITH_TIMEOUT(table->model()->rowCount() > 0 && table->model()->rowCount() < originalCount, 1000);
    searchEdit->clear();
    QTRY_COMPARE_WITH_TIMEOUT(table->model()->rowCount(), originalCount, 1000);

    newButton->click();
    QVERIFY(nameEdit->isEnabled());
    nameEdit->setText(QStringLiteral("UI 测试产品"));
    categoryEdit->setCurrentText(QStringLiteral("自动化测试"));
    modelEdit->setText(QStringLiteral("QL-UI-TEST-001"));
    QMetaObject::invokeMethod(nameEdit, "textEdited", Q_ARG(QString, nameEdit->text()));
    QVERIFY(saveButton->isEnabled());
    saveButton->click();
    QTRY_COMPARE_WITH_TIMEOUT(table->model()->rowCount(), originalCount + 1, 1000);
    QVERIFY(dialog.catalogChanged());
    QVERIFY(!saveButton->isEnabled());

    const QString screenshotPath = qEnvironmentVariable("QLIOT_UI_SCREENSHOT");
    if (!screenshotPath.isEmpty())
        QVERIFY(dialog.grab().save(screenshotPath));
}

QTEST_MAIN(ProductManagementUiTest)
#include "test_product_management_ui.moc"

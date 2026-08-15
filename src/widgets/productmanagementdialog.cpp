#include "gucds/widgets/productmanagementdialog.h"

#include "gucds/core/deviceparameter.h"
#include "gucds/core/labviewdatabase.h"
#include "gucds/core/producttablemodel.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QFrame>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSet>
#include <QShortcut>
#include <QSignalBlocker>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QSplitter>
#include <QTableView>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>

#include <algorithm>

namespace {

constexpr int kParameterCount = 5;

bool enabledValue(const QString &value)
{
    const QString normalized = value.trimmed().toLower();
    return normalized == QStringLiteral("开") || normalized == QStringLiteral("是")
        || normalized == QStringLiteral("true") || normalized == QStringLiteral("1");
}

class ProductFilterProxyModel final : public QSortFilterProxyModel
{
public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    void setCategory(const QString &category)
    {
        if (m_category == category)
            return;
        m_category = category;
        invalidateRowsFilter();
    }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override
    {
        if (!m_category.isEmpty()) {
            const QModelIndex categoryIndex = sourceModel()->index(
                sourceRow,
                gucds::ProductTableModel::CategoryColumn,
                sourceParent);
            if (sourceModel()->data(categoryIndex).toString() != m_category)
                return false;
        }
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }

private:
    QString m_category;
};

QLineEdit *parameterNameEdit(QTableWidget *table, int row)
{
    return qobject_cast<QLineEdit *>(table ? table->cellWidget(row, 0) : nullptr);
}

QComboBox *parameterModeEdit(QTableWidget *table, int row)
{
    return qobject_cast<QComboBox *>(table ? table->cellWidget(row, 1) : nullptr);
}

QComboBox *parameterTypeEdit(QTableWidget *table, int row)
{
    return qobject_cast<QComboBox *>(table ? table->cellWidget(row, 2) : nullptr);
}

QLineEdit *parameterValueEdit(QTableWidget *table, int row)
{
    return qobject_cast<QLineEdit *>(table ? table->cellWidget(row, 3) : nullptr);
}

} // namespace

ProductManagementDialog::ProductManagementDialog(const QString &databasePath, QWidget *parent)
    : QDialog(parent)
    , m_databasePath(databasePath)
    , m_model(new gucds::ProductTableModel(this))
    , m_proxyModel(new ProductFilterProxyModel(this))
    , m_filterTimer(new QTimer(this))
{
    setWindowTitle(tr("产品管理"));
    resize(1120, 720);
    setMinimumSize(900, 620);
    setModal(true);

    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1);
    m_proxyModel->setDynamicSortFilter(true);
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(160);

    buildUi();
    if (loadRecords() && m_proxyModel->rowCount() > 0)
        m_table->selectRow(0);
}

bool ProductManagementDialog::requestAuthorization(QWidget *parent)
{
    const QByteArray configuredPassword = qgetenv("GUCDS_PRODUCT_PASSWORD");
    static const QList<QByteArray> legacyPasswordHashes = {
        QByteArrayLiteral("339d4b825de7031749fc41a53db924c304e3516c80cfcc3f915913c31cbdf05b"),
        QByteArrayLiteral("f4a3522ab25d29ba72d50cf901397bbce5e432bd5140b3fa217c74b50e6eca6d"),
    };

    for (int attempt = 0; attempt < 3; ++attempt) {
        QInputDialog dialog(parent);
        dialog.setWindowTitle(tr("产品管理权限"));
        dialog.setLabelText(tr("请输入产品管理密码："));
        dialog.setTextEchoMode(QLineEdit::Password);
        dialog.setOkButtonText(tr("确定"));
        dialog.setCancelButtonText(tr("取消"));
        if (dialog.exec() != QDialog::Accepted)
            return false;

        const QByteArray password = dialog.textValue().toUtf8();
        const QByteArray passwordHash = QCryptographicHash::hash(password, QCryptographicHash::Sha256).toHex();
        const bool accepted = configuredPassword.isEmpty()
            ? legacyPasswordHashes.contains(passwordHash)
            : password == configuredPassword;
        if (accepted)
            return true;

        QMessageBox::warning(parent,
                             tr("产品管理权限"),
                             attempt < 2 ? tr("密码不正确，请重试。")
                                         : tr("密码连续三次不正确，已取消进入产品管理。"));
    }
    return false;
}

bool ProductManagementDialog::catalogChanged() const
{
    return m_catalogChanged;
}

void ProductManagementDialog::reject()
{
    if (resolveUnsavedChanges())
        QDialog::reject();
}

void ProductManagementDialog::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(8);

    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);
    rootLayout->addWidget(splitter, 1);

    auto *catalogPanel = new QWidget(splitter);
    auto *catalogLayout = new QVBoxLayout(catalogPanel);
    catalogLayout->setContentsMargins(0, 0, 4, 0);
    catalogLayout->setSpacing(6);

    auto *filterLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit(catalogPanel);
    m_searchEdit->setObjectName(QStringLiteral("productSearchEdit"));
    m_searchEdit->setPlaceholderText(tr("搜索名称、类别、型号或数据项"));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setAccessibleName(tr("搜索产品"));
    m_filterCategory = new QComboBox(catalogPanel);
    m_filterCategory->setObjectName(QStringLiteral("productCategoryFilter"));
    m_filterCategory->setMinimumWidth(150);
    filterLayout->addWidget(m_searchEdit, 1);
    filterLayout->addWidget(m_filterCategory);
    catalogLayout->addLayout(filterLayout);

    m_table = new QTableView(catalogPanel);
    m_table->setObjectName(QStringLiteral("productTable"));
    m_table->setModel(m_proxyModel);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setSortingEnabled(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(24);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->horizontalHeader()->setSectionResizeMode(gucds::ProductTableModel::NameColumn, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(gucds::ProductTableModel::ModelColumn, QHeaderView::Stretch);
    m_table->setColumnWidth(gucds::ProductTableModel::CategoryColumn, 130);
    m_table->setColumnWidth(gucds::ProductTableModel::CommunicationColumn, 80);
    m_table->setColumnWidth(gucds::ProductTableModel::ParameterColumn, 80);
    m_table->setColumnHidden(gucds::ProductTableModel::DataColumn, true);
    m_table->setColumnHidden(gucds::ProductTableModel::CalibrationColumn, true);
    catalogLayout->addWidget(m_table, 1);

    auto *catalogActions = new QHBoxLayout;
    auto *newButton = new QPushButton(tr("新建"), catalogPanel);
    newButton->setObjectName(QStringLiteral("newProductButton"));
    auto *duplicateButton = new QPushButton(tr("复制"), catalogPanel);
    duplicateButton->setObjectName(QStringLiteral("duplicateProductButton"));
    m_deleteButton = new QPushButton(tr("删除"), catalogPanel);
    m_deleteButton->setObjectName(QStringLiteral("deleteProductButton"));
    m_deleteButton->setEnabled(false);
    m_resultLabel = new QLabel(catalogPanel);
    catalogActions->addWidget(newButton);
    catalogActions->addWidget(duplicateButton);
    catalogActions->addWidget(m_deleteButton);
    catalogActions->addStretch();
    catalogActions->addWidget(m_resultLabel);
    catalogLayout->addLayout(catalogActions);

    auto *editorScroll = new QScrollArea(splitter);
    editorScroll->setWidgetResizable(true);
    editorScroll->setFrameShape(QFrame::NoFrame);
    auto *editorPanel = new QWidget(editorScroll);
    auto *editorLayout = new QVBoxLayout(editorPanel);
    editorLayout->setContentsMargins(6, 0, 0, 0);
    editorLayout->setSpacing(8);

    m_editorTitle = new QLabel(tr("选择一个产品进行编辑"), editorPanel);
    QFont titleFont = m_editorTitle->font();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Medium);
    m_editorTitle->setFont(titleFont);
    editorLayout->addWidget(m_editorTitle);

    auto *basicGroup = new QGroupBox(tr("基本信息"), editorPanel);
    auto *basicForm = new QFormLayout(basicGroup);
    basicForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    basicForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    m_nameEdit = new QLineEdit(basicGroup);
    m_nameEdit->setObjectName(QStringLiteral("productNameEdit"));
    m_categoryEdit = new QComboBox(basicGroup);
    m_categoryEdit->setObjectName(QStringLiteral("productCategoryEdit"));
    m_categoryEdit->setEditable(true);
    m_categoryEdit->setInsertPolicy(QComboBox::NoInsert);
    m_modelEdit = new QLineEdit(basicGroup);
    m_modelEdit->setObjectName(QStringLiteral("productModelEdit"));
    basicForm->addRow(tr("产品名称 *"), m_nameEdit);
    basicForm->addRow(tr("产品类别 *"), m_categoryEdit);
    basicForm->addRow(tr("规格型号 *"), m_modelEdit);
    editorLayout->addWidget(basicGroup);

    auto *dataGroup = new QGroupBox(tr("数据名称"), editorPanel);
    auto *dataGrid = new QGridLayout(dataGroup);
    m_dataEdits.reserve(5);
    for (int index = 0; index < 5; ++index) {
        auto *label = new QLabel(tr("数据%1").arg(index + 1), dataGroup);
        auto *edit = new QLineEdit(dataGroup);
        edit->setPlaceholderText(index == 4 ? tr("可选扩展数据项") : QString());
        m_dataEdits.append(edit);
        const int row = index / 2;
        const int column = (index % 2) * 2;
        dataGrid->addWidget(label, row, column);
        dataGrid->addWidget(edit, row, column + 1);
    }
    dataGrid->setColumnStretch(1, 1);
    dataGrid->setColumnStretch(3, 1);
    editorLayout->addWidget(dataGroup);

    auto *communicationGroup = new QGroupBox(tr("通信与标定"), editorPanel);
    auto *communicationLayout = new QHBoxLayout(communicationGroup);
    m_modbusCheck = new QCheckBox(QStringLiteral("Modbus"), communicationGroup);
    m_loraCheck = new QCheckBox(QStringLiteral("LoRa"), communicationGroup);
    m_dtuCheck = new QCheckBox(QStringLiteral("DTU"), communicationGroup);
    m_calibrationPoints = new QSpinBox(communicationGroup);
    m_calibrationPoints->setRange(0, 999);
    m_calibrationPoints->setSuffix(tr(" 点"));
    communicationLayout->addWidget(m_modbusCheck);
    communicationLayout->addWidget(m_loraCheck);
    communicationLayout->addWidget(m_dtuCheck);
    communicationLayout->addStretch();
    communicationLayout->addWidget(new QLabel(tr("标定点数"), communicationGroup));
    communicationLayout->addWidget(m_calibrationPoints);
    editorLayout->addWidget(communicationGroup);

    auto *parameterGroup = new QGroupBox(tr("传感器参数定义"), editorPanel);
    auto *parameterLayout = new QVBoxLayout(parameterGroup);
    m_parameterTable = new QTableWidget(kParameterCount, 4, parameterGroup);
    m_parameterTable->setObjectName(QStringLiteral("productParameterTable"));
    m_parameterTable->setHorizontalHeaderLabels({
        tr("参数名称"),
        tr("输入方式"),
        tr("数据类型"),
        tr("默认值 / 菜单项"),
    });
    m_parameterTable->verticalHeader()->setVisible(true);
    m_parameterTable->verticalHeader()->setDefaultSectionSize(28);
    m_parameterTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_parameterTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_parameterTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_parameterTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_parameterTable->setMinimumHeight(205);
    for (int row = 0; row < kParameterCount; ++row) {
        m_parameterTable->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
        auto *nameEdit = new QLineEdit(m_parameterTable);
        auto *modeEdit = new QComboBox(m_parameterTable);
        modeEdit->addItem(tr("字符"), QStringLiteral("字符"));
        modeEdit->addItem(tr("菜单"), QStringLiteral("菜单"));
        auto *typeEdit = new QComboBox(m_parameterTable);
        typeEdit->addItem(tr("整数"), QStringLiteral("整数"));
        typeEdit->addItem(tr("浮点"), QStringLiteral("浮点"));
        typeEdit->addItem(tr("字符"), QStringLiteral("字符"));
        auto *valueEdit = new QLineEdit(m_parameterTable);
        m_parameterTable->setCellWidget(row, 0, nameEdit);
        m_parameterTable->setCellWidget(row, 1, modeEdit);
        m_parameterTable->setCellWidget(row, 2, typeEdit);
        m_parameterTable->setCellWidget(row, 3, valueEdit);

        connect(nameEdit, &QLineEdit::textEdited, this, [this] { markDirty(); });
        connect(typeEdit, &QComboBox::currentTextChanged, this, [this] { markDirty(); });
        connect(valueEdit, &QLineEdit::textEdited, this, [this] { markDirty(); });
        connect(modeEdit, &QComboBox::currentIndexChanged, this, [this, modeEdit, valueEdit] {
            valueEdit->setPlaceholderText(modeEdit->currentData().toString() == QStringLiteral("菜单")
                                              ? tr("多个菜单项用英文逗号分隔")
                                              : tr("默认值"));
            markDirty();
        });
    }
    parameterLayout->addWidget(m_parameterTable);
    editorLayout->addWidget(parameterGroup);
    editorLayout->addStretch();
    editorScroll->setWidget(editorPanel);

    splitter->addWidget(catalogPanel);
    splitter->addWidget(editorScroll);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
    splitter->setSizes({650, 470});

    auto *bottomActions = new QHBoxLayout;
    m_revertButton = new QPushButton(tr("撤销修改"), this);
    m_revertButton->setObjectName(QStringLiteral("revertProductButton"));
    m_saveButton = new QPushButton(tr("保存产品"), this);
    m_saveButton->setObjectName(QStringLiteral("saveProductButton"));
    auto *closeButton = new QPushButton(tr("关闭"), this);
    m_revertButton->setEnabled(false);
    m_saveButton->setEnabled(false);
    m_saveButton->setDefault(true);
    bottomActions->addStretch();
    bottomActions->addWidget(m_revertButton);
    bottomActions->addWidget(m_saveButton);
    bottomActions->addWidget(closeButton);
    rootLayout->addLayout(bottomActions);

    connect(m_searchEdit, &QLineEdit::textChanged, m_filterTimer, qOverload<>(&QTimer::start));
    connect(m_filterTimer, &QTimer::timeout, this, [this] {
        m_proxyModel->setFilterFixedString(m_searchEdit->text().trimmed());
        updateResultCount();
    });
    connect(m_filterCategory, &QComboBox::currentIndexChanged, this, [this] {
        static_cast<ProductFilterProxyModel *>(m_proxyModel)->setCategory(m_filterCategory->currentData().toString());
        updateResultCount();
    });
    connect(m_table->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ProductManagementDialog::handleCurrentProductChanged);
    connect(newButton, &QPushButton::clicked, this, &ProductManagementDialog::createProduct);
    connect(duplicateButton, &QPushButton::clicked, this, &ProductManagementDialog::duplicateProduct);
    connect(m_deleteButton, &QPushButton::clicked, this, &ProductManagementDialog::deleteProduct);
    connect(m_saveButton, &QPushButton::clicked, this, [this] { saveProduct(); });
    connect(m_revertButton, &QPushButton::clicked, this, &ProductManagementDialog::revertProduct);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);

    connect(m_nameEdit, &QLineEdit::textEdited, this, [this] { markDirty(); });
    connect(m_categoryEdit, &QComboBox::currentTextChanged, this, [this] { markDirty(); });
    connect(m_modelEdit, &QLineEdit::textEdited, this, [this] { markDirty(); });
    for (QLineEdit *edit : m_dataEdits)
        connect(edit, &QLineEdit::textEdited, this, [this] { markDirty(); });
    connect(m_modbusCheck, &QCheckBox::checkStateChanged, this, [this] { markDirty(); });
    connect(m_loraCheck, &QCheckBox::checkStateChanged, this, [this] { markDirty(); });
    connect(m_dtuCheck, &QCheckBox::checkStateChanged, this, [this] { markDirty(); });
    connect(m_calibrationPoints, &QSpinBox::valueChanged, this, [this] { markDirty(); });

    auto *newShortcut = new QShortcut(QKeySequence::New, this);
    auto *saveShortcut = new QShortcut(QKeySequence::Save, this);
    auto *deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    connect(newShortcut, &QShortcut::activated, this, &ProductManagementDialog::createProduct);
    connect(saveShortcut, &QShortcut::activated, this, [this] { saveProduct(); });
    connect(deleteShortcut, &QShortcut::activated, this, &ProductManagementDialog::deleteProduct);

    setEditorEnabled(false);
}

bool ProductManagementDialog::loadRecords()
{
    QString errorMessage;
    const QVector<gucds::DeviceRecord> records = gucds::LabviewDatabase::loadDeviceRecords(m_databasePath, &errorMessage);
    if (!errorMessage.isEmpty()) {
        QMessageBox::critical(this, tr("产品管理"), tr("读取产品库失败：%1").arg(errorMessage));
        return false;
    }

    m_loading = true;
    m_model->setRecords(records);
    rebuildCategoryLists();
    m_loading = false;
    updateResultCount();
    return true;
}

void ProductManagementDialog::rebuildCategoryLists()
{
    QSet<QString> categorySet;
    for (const gucds::DeviceRecord &record : m_model->records()) {
        const QString category = record.category.trimmed();
        if (!category.isEmpty())
            categorySet.insert(category);
    }
    QStringList categories(categorySet.cbegin(), categorySet.cend());
    categories.sort(Qt::CaseInsensitive);

    const QString filterValue = m_filterCategory->currentData().toString();
    const QString editorValue = m_categoryEdit->currentText();
    const QSignalBlocker filterBlocker(m_filterCategory);
    const QSignalBlocker editorBlocker(m_categoryEdit);
    m_filterCategory->clear();
    m_filterCategory->addItem(tr("全部类别"), QString());
    for (const QString &category : categories)
        m_filterCategory->addItem(category, category);
    const int filterIndex = m_filterCategory->findData(filterValue);
    m_filterCategory->setCurrentIndex(filterIndex >= 0 ? filterIndex : 0);
    m_categoryEdit->clear();
    m_categoryEdit->addItems(categories);
    m_categoryEdit->setCurrentText(editorValue);
}

void ProductManagementDialog::updateResultCount()
{
    m_resultLabel->setText(tr("显示 %1 / %2 项").arg(m_proxyModel->rowCount()).arg(m_model->rowCount()));
}

void ProductManagementDialog::updateEditorTitle()
{
    QString title;
    if (m_currentRecord.databaseId > 0) {
        title = tr("编辑产品：%1").arg(m_currentRecord.model.trimmed());
    } else if (!m_editorMode.isEmpty()) {
        title = m_editorMode;
    } else {
        title = tr("选择一个产品进行编辑");
    }
    if (m_dirty)
        title.append(QStringLiteral(" *"));
    m_editorTitle->setText(title);
}

void ProductManagementDialog::selectProduct(qint64 databaseId)
{
    const int sourceRow = m_model->rowForDatabaseId(databaseId);
    if (sourceRow < 0)
        return;
    const QModelIndex proxyIndex = m_proxyModel->mapFromSource(m_model->index(sourceRow, 0));
    if (!proxyIndex.isValid())
        return;

    const QSignalBlocker blocker(m_table->selectionModel());
    m_table->setCurrentIndex(proxyIndex);
    m_table->selectRow(proxyIndex.row());
    m_table->scrollTo(proxyIndex);
}

void ProductManagementDialog::handleCurrentProductChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (m_loading)
        return;
    if (!resolveUnsavedChanges()) {
        const QSignalBlocker blocker(m_table->selectionModel());
        if (previous.isValid()) {
            m_table->setCurrentIndex(previous);
            m_table->selectRow(previous.row());
        } else {
            m_table->clearSelection();
        }
        return;
    }

    if (!current.isValid()) {
        clearEditor();
        return;
    }
    const QModelIndex sourceIndex = m_proxyModel->mapToSource(current);
    loadRecord(m_model->recordAt(sourceIndex.row()));
}

void ProductManagementDialog::loadRecord(const gucds::DeviceRecord &record, const QString &modeText)
{
    m_loading = true;
    m_currentRecord = record;
    m_editorMode = modeText;
    m_nameEdit->setText(record.name);
    m_categoryEdit->setCurrentText(record.category);
    m_modelEdit->setText(record.model);
    const QStringList dataValues = {record.data1, record.data2, record.data3, record.data4, record.data5};
    for (int index = 0; index < m_dataEdits.size(); ++index)
        m_dataEdits.at(index)->setText(dataValues.value(index));
    m_modbusCheck->setChecked(enabledValue(record.modbus));
    m_loraCheck->setChecked(enabledValue(record.lora));
    m_dtuCheck->setChecked(enabledValue(record.dtu));
    m_calibrationPoints->setValue(record.calibrationPoints);

    for (int row = 0; row < kParameterCount; ++row) {
        const gucds::DeviceParameterDefinition definition =
            gucds::parseDeviceParameterDefinition(gucds::deviceParameterDefinition(record, row + 1));
        parameterNameEdit(m_parameterTable, row)->setText(definition.name);
        QComboBox *modeEdit = parameterModeEdit(m_parameterTable, row);
        QComboBox *typeEdit = parameterTypeEdit(m_parameterTable, row);
        const QString mode = definition.editorMode.isEmpty() ? QStringLiteral("字符") : definition.editorMode;
        const QString type = definition.valueType.isEmpty() ? QStringLiteral("字符") : definition.valueType;
        modeEdit->setCurrentIndex((std::max)(0, modeEdit->findData(mode)));
        typeEdit->setCurrentIndex((std::max)(0, typeEdit->findData(type)));
        parameterValueEdit(m_parameterTable, row)->setText(definition.valueText);
    }

    m_dirty = false;
    setEditorEnabled(true);
    m_deleteButton->setEnabled(record.databaseId > 0);
    m_saveButton->setEnabled(false);
    m_revertButton->setEnabled(false);
    updateEditorTitle();
    m_loading = false;
}

void ProductManagementDialog::clearEditor()
{
    m_loading = true;
    m_currentRecord = {};
    m_editorMode.clear();
    m_nameEdit->clear();
    m_categoryEdit->setCurrentIndex(-1);
    m_modelEdit->clear();
    for (QLineEdit *edit : m_dataEdits)
        edit->clear();
    m_modbusCheck->setChecked(false);
    m_loraCheck->setChecked(false);
    m_dtuCheck->setChecked(false);
    m_calibrationPoints->setValue(0);
    for (int row = 0; row < kParameterCount; ++row) {
        parameterNameEdit(m_parameterTable, row)->clear();
        parameterModeEdit(m_parameterTable, row)->setCurrentIndex(0);
        parameterTypeEdit(m_parameterTable, row)->setCurrentIndex(0);
        parameterValueEdit(m_parameterTable, row)->clear();
    }
    m_dirty = false;
    setEditorEnabled(false);
    m_deleteButton->setEnabled(false);
    m_saveButton->setEnabled(false);
    m_revertButton->setEnabled(false);
    updateEditorTitle();
    m_loading = false;
}

gucds::DeviceRecord ProductManagementDialog::editorRecord() const
{
    gucds::DeviceRecord record = m_currentRecord;
    record.name = m_nameEdit->text().trimmed();
    record.category = m_categoryEdit->currentText().trimmed();
    record.model = m_modelEdit->text().trimmed();
    if (m_dataEdits.size() >= 5) {
        record.data1 = m_dataEdits.at(0)->text().trimmed();
        record.data2 = m_dataEdits.at(1)->text().trimmed();
        record.data3 = m_dataEdits.at(2)->text().trimmed();
        record.data4 = m_dataEdits.at(3)->text().trimmed();
        record.data5 = m_dataEdits.at(4)->text().trimmed();
    }
    record.modbus = m_modbusCheck->isChecked() ? QStringLiteral("开") : QStringLiteral("关");
    record.lora = m_loraCheck->isChecked() ? QStringLiteral("开") : QStringLiteral("关");
    record.dtu = m_dtuCheck->isChecked() ? QStringLiteral("开") : QStringLiteral("关");
    record.calibrationPoints = m_calibrationPoints->value();
    for (int row = 0; row < kParameterCount; ++row) {
        gucds::DeviceParameterDefinition definition;
        definition.name = parameterNameEdit(m_parameterTable, row)->text();
        definition.editorMode = parameterModeEdit(m_parameterTable, row)->currentData().toString();
        definition.valueType = parameterTypeEdit(m_parameterTable, row)->currentData().toString();
        definition.valueText = parameterValueEdit(m_parameterTable, row)->text();
        gucds::setDeviceParameterDefinition(&record,
                                            row + 1,
                                            gucds::formatDeviceParameterDefinition(definition));
    }
    return record;
}

void ProductManagementDialog::setEditorEnabled(bool enabled)
{
    m_nameEdit->setEnabled(enabled);
    m_categoryEdit->setEnabled(enabled);
    m_modelEdit->setEnabled(enabled);
    for (QLineEdit *edit : m_dataEdits)
        edit->setEnabled(enabled);
    m_modbusCheck->setEnabled(enabled);
    m_loraCheck->setEnabled(enabled);
    m_dtuCheck->setEnabled(enabled);
    m_calibrationPoints->setEnabled(enabled);
    m_parameterTable->setEnabled(enabled);
}

void ProductManagementDialog::markDirty()
{
    if (m_loading || !m_nameEdit->isEnabled())
        return;
    m_dirty = true;
    m_saveButton->setEnabled(true);
    m_revertButton->setEnabled(true);
    updateEditorTitle();
}

bool ProductManagementDialog::resolveUnsavedChanges()
{
    if (!m_dirty)
        return true;

    const QMessageBox::StandardButton answer = QMessageBox::warning(
        this,
        tr("未保存的产品修改"),
        tr("当前产品有未保存的修改。"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);
    if (answer == QMessageBox::Save)
        return saveProduct();
    if (answer == QMessageBox::Discard) {
        m_dirty = false;
        return true;
    }
    return false;
}

void ProductManagementDialog::createProduct()
{
    if (!resolveUnsavedChanges())
        return;

    gucds::DeviceRecord record;
    const QString filteredCategory = m_filterCategory->currentData().toString();
    if (!filteredCategory.isEmpty())
        record.category = filteredCategory;
    record.modbus = QStringLiteral("开");
    record.lora = QStringLiteral("关");
    record.dtu = QStringLiteral("关");
    m_loading = true;
    m_table->clearSelection();
    m_loading = false;
    loadRecord(record, tr("新建产品"));
    markDirty();
    m_nameEdit->setFocus();
}

void ProductManagementDialog::duplicateProduct()
{
    if (!resolveUnsavedChanges())
        return;
    if (m_currentRecord.databaseId <= 0) {
        QMessageBox::information(this, tr("复制产品"), tr("请先选择要复制的产品。"));
        return;
    }

    gucds::DeviceRecord copy = m_currentRecord;
    copy.databaseId = -1;
    copy.name = copy.name.trimmed() + tr(" 副本");
    copy.model = copy.model.trimmed() + QStringLiteral("-COPY");
    m_loading = true;
    m_table->clearSelection();
    m_loading = false;
    loadRecord(copy, tr("复制产品"));
    markDirty();
    m_nameEdit->selectAll();
    m_nameEdit->setFocus();
}

bool ProductManagementDialog::saveProduct()
{
    if (!m_nameEdit->isEnabled())
        return false;

    gucds::DeviceRecord record = editorRecord();
    if (record.name.isEmpty() || record.category.isEmpty() || record.model.isEmpty()) {
        QMessageBox::warning(this,
                             tr("保存产品"),
                             tr("请填写产品名称、产品类别和规格型号。"));
        return false;
    }

    QSet<QString> parameterNames;
    for (int row = 0; row < kParameterCount; ++row) {
        const gucds::DeviceParameterDefinition definition = gucds::parseDeviceParameterDefinition(
            gucds::deviceParameterDefinition(record, row + 1));
        if (!definition.isValid())
            continue;
        const QString normalizedName = definition.name.trimmed().toCaseFolded();
        if (parameterNames.contains(normalizedName)) {
            QMessageBox::warning(this,
                                 tr("保存产品"),
                                 tr("参数名称“%1”重复，请修改后再保存。").arg(definition.name));
            parameterNameEdit(m_parameterTable, row)->setFocus();
            return false;
        }
        parameterNames.insert(normalizedName);
        if (definition.editorMode == QStringLiteral("菜单") && definition.options().isEmpty()) {
            QMessageBox::warning(this,
                                 tr("保存产品"),
                                 tr("参数“%1”使用菜单输入，请至少填写一个菜单项。")
                                     .arg(definition.name));
            parameterValueEdit(m_parameterTable, row)->setFocus();
            return false;
        }
    }

    QString errorMessage;
    const bool isNew = record.databaseId <= 0;
    if (!gucds::LabviewDatabase::saveDeviceRecord(m_databasePath, &record, &errorMessage)) {
        QMessageBox::critical(this, tr("保存产品"), errorMessage);
        return false;
    }

    m_loading = true;
    if (isNew)
        m_model->addRecord(record);
    else
        m_model->updateRecord(record);
    m_currentRecord = record;
    m_editorMode.clear();
    m_dirty = false;
    m_catalogChanged = true;
    rebuildCategoryLists();
    static_cast<ProductFilterProxyModel *>(m_proxyModel)->setCategory(m_filterCategory->currentData().toString());
    updateResultCount();
    selectProduct(record.databaseId);
    m_deleteButton->setEnabled(true);
    m_saveButton->setEnabled(false);
    m_revertButton->setEnabled(false);
    updateEditorTitle();
    m_loading = false;
    return true;
}

void ProductManagementDialog::deleteProduct()
{
    if (m_currentRecord.databaseId <= 0)
        return;
    const QMessageBox::StandardButton answer = QMessageBox::warning(
        this,
        tr("删除产品"),
        tr("确定删除产品“%1 / %2”吗？\n主界面设备库中将不再提供该候选产品。")
            .arg(m_currentRecord.name, m_currentRecord.model),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);
    if (answer != QMessageBox::Yes)
        return;

    const int selectedProxyRow = m_table->currentIndex().row();
    QString errorMessage;
    const qint64 databaseId = m_currentRecord.databaseId;
    if (!gucds::LabviewDatabase::deleteDeviceRecord(m_databasePath, databaseId, &errorMessage)) {
        QMessageBox::critical(this, tr("删除产品"), errorMessage);
        return;
    }

    m_loading = true;
    m_model->removeRecord(databaseId);
    m_catalogChanged = true;
    m_dirty = false;
    rebuildCategoryLists();
    updateResultCount();
    clearEditor();
    m_loading = false;
    if (m_proxyModel->rowCount() > 0)
        m_table->selectRow((std::clamp)(selectedProxyRow, 0, m_proxyModel->rowCount() - 1));
}

void ProductManagementDialog::revertProduct()
{
    if (m_currentRecord.databaseId > 0) {
        const int row = m_model->rowForDatabaseId(m_currentRecord.databaseId);
        if (row >= 0)
            loadRecord(m_model->recordAt(row));
    } else {
        clearEditor();
    }
}

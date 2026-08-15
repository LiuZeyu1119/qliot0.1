#include "gucds/widgets/frequencytensionparameterdialog.h"

#include "gucds/core/labviewdatabase.h"

#include <QAbstractItemView>
#include <QDoubleValidator>
#include <QFont>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>

namespace {

constexpr int kColumnCount = 8;

QLocale parameterLocale()
{
    QLocale locale;
    locale.setNumberOptions(QLocale::OmitGroupSeparator | QLocale::RejectGroupSeparator);
    return locale;
}

QString formatNumber(double value)
{
    return parameterLocale().toString(value, 'g', 12);
}

QLineEdit *createNumberEdit(const QString &objectName,
                            const QString &placeholder,
                            double minimum,
                            double maximum,
                            QWidget *parent)
{
    auto *edit = new QLineEdit(parent);
    edit->setObjectName(objectName);
    edit->setPlaceholderText(placeholder);
    auto *validator = new QDoubleValidator(minimum, maximum, 12, edit);
    validator->setNotation(QDoubleValidator::StandardNotation);
    validator->setLocale(parameterLocale());
    edit->setValidator(validator);
    edit->setClearButtonEnabled(true);
    return edit;
}

void addEditorField(QGridLayout *layout,
                    int row,
                    int column,
                    const QString &labelText,
                    QLineEdit *edit,
                    QWidget *parent)
{
    auto *label = new QLabel(labelText, parent);
    label->setBuddy(edit);
    layout->addWidget(label, row * 2, column);
    layout->addWidget(edit, row * 2 + 1, column);
}

} // namespace

FrequencyTensionParameterDialog::FrequencyTensionParameterDialog(const QString &databasePath, QWidget *parent)
    : QDialog(parent)
    , m_databasePath(databasePath)
{
    setObjectName(QStringLiteral("frequencyTensionParameterDialog"));
    setWindowTitle(tr("频振索力传感器扩展参数管理器"));
    setModal(true);
    resize(1180, 650);
    setMinimumSize(900, 560);

    buildUi();
    if (loadRecords() && !m_records.isEmpty())
        selectRow(0);
}

bool FrequencyTensionParameterDialog::hasUnsavedChanges() const
{
    return m_dirty;
}

void FrequencyTensionParameterDialog::setSensorWriteAvailable(bool available)
{
    m_sensorWriteAvailable = available;
    m_saveButton->setEnabled(!m_sensorWritePending && m_dirty);
    m_writeSensorButton->setEnabled(!m_sensorWritePending
                                    && m_sensorWriteAvailable
                                    && m_table->currentRow() >= 0);
}

void FrequencyTensionParameterDialog::finishSensorWrite(
    bool success,
    const QString &message,
    const QVector<double> &readbackValues)
{
    if (!m_sensorWritePending)
        return;

    m_sensorWritePending = false;
    m_saveButton->setEnabled(m_dirty);
    m_writeSensorButton->setEnabled(m_sensorWriteAvailable && m_table->currentRow() >= 0);
    if (!success) {
        m_statusLabel->setText(tr("写入传感器失败：%1").arg(message));
        return;
    }

    const QVector<double> expected = {
        m_pendingSensorRecord.supportFactor,
        m_pendingSensorRecord.unitMass,
        m_pendingSensorRecord.cableLength,
        m_pendingSensorRecord.area,
        m_pendingSensorRecord.elasticModulus,
        m_pendingSensorRecord.inertia,
        m_pendingSensorRecord.angle,
    };
    if (readbackValues.size() < expected.size()) {
        m_statusLabel->setText(tr("设备已确认写入，但 AT,get,FVCFexppar 未返回完整的 7 个参数，无法完成回读校验。"));
        return;
    }

    for (qsizetype index = 0; index < expected.size(); ++index) {
        const double tolerance = (std::max)(0.005, std::abs(expected.at(index)) * 1.0e-6);
        if (std::abs(readbackValues.at(index) - expected.at(index)) > tolerance) {
            m_statusLabel->setText(
                tr("传感器回读值与设置值不一致：第 %1 项期望 %2，回读 %3。")
                    .arg(index + 1)
                    .arg(formatNumber(expected.at(index)), formatNumber(readbackValues.at(index))));
            return;
        }
    }

    m_statusLabel->setText(tr("扩展参数已写入传感器，且 AT,get,FVCFexppar 回读校验一致。"));
}

void FrequencyTensionParameterDialog::reject()
{
    if (resolveUnsavedChanges())
        QDialog::reject();
}

void FrequencyTensionParameterDialog::buildUi()
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(14, 12, 14, 12);
    rootLayout->setSpacing(10);

    auto *title = new QLabel(tr("频振索力传感器扩展参数管理器"), this);
    title->setObjectName(QStringLiteral("frequencyParameterTitle"));
    QFont titleFont = title->font();
    titleFont.setPointSize(12);
    titleFont.setWeight(QFont::Medium);
    title->setFont(titleFont);
    rootLayout->addWidget(title);

    auto *description = new QLabel(
        tr("“保存到本机”仅更新本地数据库；“写入传感器并校验”会发送 AT,set,exppar,...，再用 AT,get,FVCFexppar 回读确认。"), this);
    description->setObjectName(QStringLiteral("frequencyParameterDescription"));
    description->setWordWrap(true);
    rootLayout->addWidget(description);

    auto *contentLayout = new QHBoxLayout;
    contentLayout->setSpacing(10);

    auto *actionGroup = new QGroupBox(tr("参数操作"), this);
    actionGroup->setMinimumWidth(200);
    actionGroup->setMaximumWidth(240);
    auto *actionLayout = new QVBoxLayout(actionGroup);
    actionLayout->setContentsMargins(10, 14, 10, 10);
    actionLayout->setSpacing(12);

    auto *addButton = new QPushButton(tr("添加"), actionGroup);
    addButton->setObjectName(QStringLiteral("addFrequencyParameterButton"));
    m_modifyButton = new QPushButton(tr("修改"), actionGroup);
    m_modifyButton->setObjectName(QStringLiteral("modifyFrequencyParameterButton"));
    m_saveButton = new QPushButton(tr("保存到本机"), actionGroup);
    m_saveButton->setObjectName(QStringLiteral("saveFrequencyParameterButton"));
    m_writeSensorButton = new QPushButton(tr("写入传感器并校验"), actionGroup);
    m_writeSensorButton->setObjectName(QStringLiteral("writeFrequencyParameterSensorButton"));
    auto *closeButton = new QPushButton(tr("关闭"), actionGroup);
    closeButton->setObjectName(QStringLiteral("closeFrequencyParameterButton"));
    for (QPushButton *button : {addButton, m_modifyButton, m_saveButton, m_writeSensorButton, closeButton})
        button->setMinimumHeight(42);
    m_modifyButton->setEnabled(false);
    m_saveButton->setEnabled(false);
    m_writeSensorButton->setEnabled(false);
    m_saveButton->setDefault(true);
    actionLayout->addWidget(addButton);
    actionLayout->addWidget(m_modifyButton);
    actionLayout->addWidget(m_saveButton);
    actionLayout->addWidget(m_writeSensorButton);
    actionLayout->addStretch();
    actionLayout->addWidget(closeButton);
    contentLayout->addWidget(actionGroup);

    m_table = new QTableWidget(this);
    m_table->setObjectName(QStringLiteral("frequencyParameterTable"));
    m_table->setColumnCount(kColumnCount);
    m_table->setHorizontalHeaderLabels({
        tr("传感器名"),
        tr("支座系数 μ"),
        tr("单位质量 G (kg/m)"),
        tr("索长 L (m)"),
        tr("截面积 A (cm²)"),
        tr("弹性模量 E (MPa)"),
        tr("截面惯性矩 I (cm⁴)"),
        tr("水平夹角 θ (°)"),
    });
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(28);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setMinimumSectionSize(90);
    contentLayout->addWidget(m_table, 1);
    rootLayout->addLayout(contentLayout, 1);

    auto *editorGroup = new QGroupBox(tr("参数编辑"), this);
    auto *editorLayout = new QGridLayout(editorGroup);
    editorLayout->setHorizontalSpacing(12);
    editorLayout->setVerticalSpacing(6);

    m_sensorNameEdit = new QLineEdit(editorGroup);
    m_sensorNameEdit->setObjectName(QStringLiteral("frequencyParameterSensorName"));
    m_sensorNameEdit->setPlaceholderText(tr("例如 Sen01"));
    m_sensorNameEdit->setClearButtonEnabled(true);
    m_supportFactorEdit = createNumberEdit(
        QStringLiteral("frequencyParameterSupportFactor"), QStringLiteral("0.97"), 0.0, 1.0e12, editorGroup);
    m_unitMassEdit = createNumberEdit(
        QStringLiteral("frequencyParameterUnitMass"), QStringLiteral("1.101"), 0.0, 1.0e12, editorGroup);
    m_cableLengthEdit = createNumberEdit(
        QStringLiteral("frequencyParameterCableLength"), QStringLiteral("2.7"), 0.0, 1.0e12, editorGroup);
    m_areaEdit = createNumberEdit(
        QStringLiteral("frequencyParameterArea"), QStringLiteral("1.4"), 0.0, 1.0e12, editorGroup);
    m_elasticModulusEdit = createNumberEdit(
        QStringLiteral("frequencyParameterElasticModulus"), QStringLiteral("195000"), 0.0, 1.0e12, editorGroup);
    m_inertiaEdit = createNumberEdit(
        QStringLiteral("frequencyParameterInertia"), QStringLiteral("0.156"), 0.0, 1.0e12, editorGroup);
    m_angleEdit = createNumberEdit(
        QStringLiteral("frequencyParameterAngle"), QStringLiteral("0"), -360.0, 360.0, editorGroup);

    addEditorField(editorLayout, 0, 0, tr("传感器名"), m_sensorNameEdit, editorGroup);
    addEditorField(editorLayout, 0, 1, tr("支座系数 μ"), m_supportFactorEdit, editorGroup);
    addEditorField(editorLayout, 0, 2, tr("单位质量 G (kg/m)"), m_unitMassEdit, editorGroup);
    addEditorField(editorLayout, 0, 3, tr("索长 L (m)"), m_cableLengthEdit, editorGroup);
    addEditorField(editorLayout, 1, 0, tr("截面积 A (cm²)"), m_areaEdit, editorGroup);
    addEditorField(editorLayout, 1, 1, tr("弹性模量 E (MPa)"), m_elasticModulusEdit, editorGroup);
    addEditorField(editorLayout, 1, 2, tr("截面惯性矩 I (cm⁴)"), m_inertiaEdit, editorGroup);
    addEditorField(editorLayout, 1, 3, tr("水平夹角 θ (°)"), m_angleEdit, editorGroup);
    for (int column = 0; column < 4; ++column)
        editorLayout->setColumnStretch(column, 1);
    rootLayout->addWidget(editorGroup);

    m_statusLabel = new QLabel(tr("选择表格中的参数可进行修改。"), this);
    m_statusLabel->setObjectName(QStringLiteral("frequencyParameterStatus"));
    m_statusLabel->setWordWrap(true);
    rootLayout->addWidget(m_statusLabel);

    setTabOrder(m_table, m_sensorNameEdit);
    setTabOrder(m_sensorNameEdit, m_supportFactorEdit);
    setTabOrder(m_supportFactorEdit, m_unitMassEdit);
    setTabOrder(m_unitMassEdit, m_cableLengthEdit);
    setTabOrder(m_cableLengthEdit, m_areaEdit);
    setTabOrder(m_areaEdit, m_elasticModulusEdit);
    setTabOrder(m_elasticModulusEdit, m_inertiaEdit);
    setTabOrder(m_inertiaEdit, m_angleEdit);
    setTabOrder(m_angleEdit, addButton);
    setTabOrder(addButton, m_modifyButton);
    setTabOrder(m_modifyButton, m_saveButton);
    setTabOrder(m_saveButton, m_writeSensorButton);
    setTabOrder(m_writeSensorButton, closeButton);

    connect(addButton, &QPushButton::clicked, this, &FrequencyTensionParameterDialog::addParameter);
    connect(m_modifyButton, &QPushButton::clicked, this, &FrequencyTensionParameterDialog::modifyParameter);
    connect(m_saveButton, &QPushButton::clicked, this, [this] { saveParameters(false); });
    connect(m_writeSensorButton,
            &QPushButton::clicked,
            this,
            &FrequencyTensionParameterDialog::writeCurrentParameterToSensor);
    connect(closeButton, &QPushButton::clicked, this, &FrequencyTensionParameterDialog::reject);
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int currentRow) {
        loadEditor(currentRow);
    });
}

bool FrequencyTensionParameterDialog::loadRecords()
{
    QString errorMessage;
    m_records = gucds::LabviewDatabase::loadFrequencyTensionParameters(m_databasePath, &errorMessage);
    if (!errorMessage.isEmpty()) {
        QMessageBox::critical(this, tr("读取参数失败"), errorMessage);
        return false;
    }
    rebuildTable();
    setDirty(false);
    m_statusLabel->setText(tr("已加载 %1 组本地参数。").arg(m_records.size()));
    return true;
}

void FrequencyTensionParameterDialog::rebuildTable()
{
    m_table->setRowCount(m_records.size());
    for (int row = 0; row < m_records.size(); ++row) {
        const gucds::FrequencyTensionParameterRecord &record = m_records.at(row);
        const QStringList values = {
            record.sensorName,
            formatNumber(record.supportFactor),
            formatNumber(record.unitMass),
            formatNumber(record.cableLength),
            formatNumber(record.area),
            formatNumber(record.elasticModulus),
            formatNumber(record.inertia),
            formatNumber(record.angle),
        };
        for (int column = 0; column < values.size(); ++column) {
            auto *item = new QTableWidgetItem(values.at(column));
            if (column > 0)
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            m_table->setItem(row, column, item);
        }
    }
}

void FrequencyTensionParameterDialog::selectRow(int row)
{
    if (row < 0 || row >= m_records.size()) {
        m_table->clearSelection();
        clearEditor();
        return;
    }
    m_table->setCurrentCell(row, 0);
    m_table->selectRow(row);
    if (QTableWidgetItem *item = m_table->item(row, 0))
        m_table->scrollToItem(item);
    loadEditor(row);
}

void FrequencyTensionParameterDialog::loadEditor(int row)
{
    if (row < 0 || row >= m_records.size()) {
        m_modifyButton->setEnabled(false);
        return;
    }
    const gucds::FrequencyTensionParameterRecord &record = m_records.at(row);
    m_sensorNameEdit->setText(record.sensorName);
    m_supportFactorEdit->setText(formatNumber(record.supportFactor));
    m_unitMassEdit->setText(formatNumber(record.unitMass));
    m_cableLengthEdit->setText(formatNumber(record.cableLength));
    m_areaEdit->setText(formatNumber(record.area));
    m_elasticModulusEdit->setText(formatNumber(record.elasticModulus));
    m_inertiaEdit->setText(formatNumber(record.inertia));
    m_angleEdit->setText(formatNumber(record.angle));
    m_modifyButton->setEnabled(true);
    m_writeSensorButton->setEnabled(m_sensorWriteAvailable && !m_sensorWritePending);
    m_statusLabel->setText(tr("正在编辑：%1").arg(record.sensorName));
}

void FrequencyTensionParameterDialog::clearEditor()
{
    for (QLineEdit *edit : {m_sensorNameEdit,
                            m_supportFactorEdit,
                            m_unitMassEdit,
                            m_cableLengthEdit,
                            m_areaEdit,
                            m_elasticModulusEdit,
                            m_inertiaEdit,
                            m_angleEdit}) {
        edit->clear();
    }
    m_modifyButton->setEnabled(false);
    m_writeSensorButton->setEnabled(false);
}

bool FrequencyTensionParameterDialog::readEditor(gucds::FrequencyTensionParameterRecord *record)
{
    if (!record)
        return false;
    record->sensorName = m_sensorNameEdit->text().trimmed();
    if (record->sensorName.isEmpty()) {
        QMessageBox::warning(this, tr("参数校验"), tr("传感器名称不能为空。"));
        m_sensorNameEdit->setFocus();
        return false;
    }
    return parseNumber(m_supportFactorEdit, tr("支座系数 μ"), true, &record->supportFactor)
        && parseNumber(m_unitMassEdit, tr("单位质量 G"), true, &record->unitMass)
        && parseNumber(m_cableLengthEdit, tr("索长 L"), true, &record->cableLength)
        && parseNumber(m_areaEdit, tr("截面积 A"), true, &record->area)
        && parseNumber(m_elasticModulusEdit, tr("弹性模量 E"), true, &record->elasticModulus)
        && parseNumber(m_inertiaEdit, tr("截面惯性矩 I"), true, &record->inertia)
        && parseNumber(m_angleEdit, tr("水平夹角 θ"), false, &record->angle);
}

bool FrequencyTensionParameterDialog::parseNumber(
    QLineEdit *edit,
    const QString &label,
    bool positive,
    double *value)
{
    bool ok = false;
    double parsed = parameterLocale().toDouble(edit->text().trimmed(), &ok);
    if (!ok) {
        QLocale fallback = QLocale::c();
        fallback.setNumberOptions(QLocale::OmitGroupSeparator | QLocale::RejectGroupSeparator);
        parsed = fallback.toDouble(edit->text().trimmed(), &ok);
    }
    if (!ok || !std::isfinite(parsed) || (positive && parsed <= 0.0)) {
        QMessageBox::warning(
            this,
            tr("参数校验"),
            positive ? tr("%1 必须是大于 0 的有效数字。").arg(label)
                     : tr("%1 必须是有效数字。").arg(label));
        edit->selectAll();
        edit->setFocus();
        return false;
    }
    *value = parsed;
    return true;
}

bool FrequencyTensionParameterDialog::sensorNameExists(const QString &sensorName, int excludedRow) const
{
    for (int row = 0; row < m_records.size(); ++row) {
        if (row != excludedRow
            && m_records.at(row).sensorName.trimmed().compare(sensorName.trimmed(), Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

void FrequencyTensionParameterDialog::addParameter()
{
    gucds::FrequencyTensionParameterRecord record;
    if (!readEditor(&record))
        return;
    if (sensorNameExists(record.sensorName)) {
        QMessageBox::warning(this,
                             tr("添加参数"),
                             tr("传感器名称“%1”已经存在。请修改现有记录或使用其他名称。")
                                 .arg(record.sensorName));
        m_sensorNameEdit->setFocus();
        return;
    }
    m_records.append(record);
    rebuildTable();
    selectRow(m_records.size() - 1);
    setDirty(true);
    m_statusLabel->setText(tr("已添加参数“%1”，请点击保存写入数据库。").arg(record.sensorName));
}

void FrequencyTensionParameterDialog::modifyParameter()
{
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_records.size()) {
        QMessageBox::information(this, tr("修改参数"), tr("请先选择要修改的参数。"));
        return;
    }
    gucds::FrequencyTensionParameterRecord record = m_records.at(row);
    if (!readEditor(&record))
        return;
    if (sensorNameExists(record.sensorName, row)) {
        QMessageBox::warning(this,
                             tr("修改参数"),
                             tr("传感器名称“%1”已经存在。请使用其他名称。").arg(record.sensorName));
        m_sensorNameEdit->setFocus();
        return;
    }
    m_records[row] = record;
    rebuildTable();
    selectRow(row);
    setDirty(true);
    m_statusLabel->setText(tr("已修改参数“%1”，请点击保存写入数据库。").arg(record.sensorName));
}

void FrequencyTensionParameterDialog::writeCurrentParameterToSensor()
{
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_records.size()) {
        QMessageBox::information(this, tr("写入传感器"), tr("请先选择要写入传感器的参数。"));
        return;
    }

    gucds::FrequencyTensionParameterRecord record = m_records.at(row);
    if (!readEditor(&record))
        return;
    if (sensorNameExists(record.sensorName, row)) {
        QMessageBox::warning(this,
                             tr("写入传感器"),
                             tr("传感器名称“%1”已经存在。请使用其他名称。").arg(record.sensorName));
        return;
    }

    m_records[row] = record;
    setDirty(true);
    if (!saveParameters(false))
        return;
    selectRow(row);
    startSensorWrite(record);
}

bool FrequencyTensionParameterDialog::saveParameters(bool offerSensorWrite)
{
    const int selectedRow = m_table->currentRow();
    QString errorMessage;
    if (!gucds::LabviewDatabase::saveFrequencyTensionParameters(
            m_databasePath, &m_records, &errorMessage)) {
        QMessageBox::critical(this, tr("保存参数失败"), errorMessage);
        return false;
    }
    setDirty(false);
    rebuildTable();
    if (!m_records.isEmpty())
        selectRow((qBound)(0, selectedRow, m_records.size() - 1));
    m_statusLabel->setText(tr("参数已保存到本机数据库。"));

    if (offerSensorWrite && m_sensorWriteAvailable
        && selectedRow >= 0 && selectedRow < m_records.size()) {
        const QMessageBox::StandardButton answer = QMessageBox::question(
            this,
            tr("写入传感器"),
            tr("本地参数已保存。是否将当前参数写入传感器？\n\n"
               "将发送 AT,set,exppar,...，随后使用 AT,get,FVCFexppar 回读校验。"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);
        if (answer == QMessageBox::Yes)
            startSensorWrite(m_records.at(selectedRow));
    }
    return true;
}

void FrequencyTensionParameterDialog::startSensorWrite(
    const gucds::FrequencyTensionParameterRecord &record)
{
    if (m_sensorWritePending)
        return;
    m_pendingSensorRecord = record;
    m_sensorWritePending = true;
    m_saveButton->setEnabled(false);
    m_writeSensorButton->setEnabled(false);
    m_statusLabel->setText(tr("正在写入传感器并回读校验，请稍候……"));
    Q_EMIT sensorWriteRequested(record);
}

bool FrequencyTensionParameterDialog::resolveUnsavedChanges()
{
    if (!m_dirty)
        return true;
    const QMessageBox::StandardButton answer = QMessageBox::warning(
        this,
        tr("未保存的参数修改"),
        tr("本地参数列表有未保存的修改。"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save);
    if (answer == QMessageBox::Save)
        return saveParameters(false);
    return answer == QMessageBox::Discard;
}

void FrequencyTensionParameterDialog::setDirty(bool dirty)
{
    m_dirty = dirty;
    m_saveButton->setEnabled(!m_sensorWritePending && dirty);
    m_writeSensorButton->setEnabled(!m_sensorWritePending
                                    && m_sensorWriteAvailable
                                    && m_table->currentRow() >= 0);
    setWindowTitle(tr("频振索力传感器扩展参数管理器")
                   + (dirty ? QStringLiteral(" *") : QString()));
}

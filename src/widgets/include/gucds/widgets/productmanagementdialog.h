#pragma once

#include "gucds/core/records.h"

#include <QDialog>
#include <QVector>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QModelIndex;
class QPushButton;
class QSortFilterProxyModel;
class QSpinBox;
class QTableView;
class QTableWidget;
class QTimer;
class QWidget;

namespace gucds {
class ProductTableModel;
}

class ProductManagementDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ProductManagementDialog)

public:
    explicit ProductManagementDialog(const QString &databasePath, QWidget *parent = nullptr);

    static bool requestAuthorization(QWidget *parent = nullptr);
    bool catalogChanged() const;

protected:
    void reject() override;

private:
    void buildUi();
    bool loadRecords();
    void rebuildCategoryLists();
    void updateResultCount();
    void updateEditorTitle();
    void selectProduct(qint64 databaseId);
    void handleCurrentProductChanged(const QModelIndex &current, const QModelIndex &previous);
    void loadRecord(const gucds::DeviceRecord &record, const QString &modeText = {});
    void clearEditor();
    gucds::DeviceRecord editorRecord() const;
    void setEditorEnabled(bool enabled);
    void markDirty();
    bool resolveUnsavedChanges();
    void createProduct();
    void duplicateProduct();
    bool saveProduct();
    void deleteProduct();
    void revertProduct();

    QString m_databasePath;
    gucds::ProductTableModel *m_model = nullptr;
    QSortFilterProxyModel *m_proxyModel = nullptr;
    QTableView *m_table = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QComboBox *m_filterCategory = nullptr;
    QLabel *m_resultLabel = nullptr;
    QLabel *m_editorTitle = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QComboBox *m_categoryEdit = nullptr;
    QLineEdit *m_modelEdit = nullptr;
    QVector<QLineEdit *> m_dataEdits;
    QCheckBox *m_modbusCheck = nullptr;
    QCheckBox *m_loraCheck = nullptr;
    QCheckBox *m_dtuCheck = nullptr;
    QSpinBox *m_calibrationPoints = nullptr;
    QTableWidget *m_parameterTable = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_revertButton = nullptr;
    QPushButton *m_deleteButton = nullptr;
    QTimer *m_filterTimer = nullptr;
    gucds::DeviceRecord m_currentRecord;
    QString m_editorMode;
    bool m_loading = false;
    bool m_dirty = false;
    bool m_catalogChanged = false;
};

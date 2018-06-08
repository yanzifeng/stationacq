#ifndef UPLOADDIALOG_H
#define UPLOADDIALOG_H

#include <QDialog>
#include "configmanagement.h"

namespace Ui {
class UploadDialog;
}

class UploadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UploadDialog(QWidget *parent = 0);
    ~UploadDialog();
private:
    void initTable();
    void loadTableData();
    void saveUnsendFiles();
    QStringList getFileProperty(QString filename);
private slots:
    void updateSelection(int);
    void selectAll();
    void diselectAll();
    void upload();
    void setFlagFalse();
public slots:
    void updateTable(QString);
signals:
    void doSelect(bool);
    void uploadFile(QString);
private:
    Ui::UploadDialog *ui;
    QStringList mList,mFileNameList;
    ConfigManagement mConfig;
    QList<bool> mSelection;
    QList<int> pSelectIndex;
    bool mFlag;
};

#endif // UPLOADDIALOG_H

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QSettings>
#include <QFile>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();
private slots:
    void saveAndExit();
    void chooseFolder();
private:
    void readConfig();
    void saveConfig();
    void listSerialPort();
private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H

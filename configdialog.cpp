#include "configdialog.h"
#include "ui_configdialog.h"
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QTextCodec>

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);  //Qt::WindowMinimizeButtonHint
    listSerialPort();
    readConfig();
    connect(ui->pushButton_save,SIGNAL(clicked(bool)),this,SLOT(saveAndExit()));
    connect(ui->pushButton_2,SIGNAL(clicked(bool)),this,SLOT(chooseFolder()));
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::readConfig()
{
    if(QFile::exists("ConfigFiles/Config.ini"))
    {
        QSettings setting("ConfigFiles/Config.ini",QSettings::IniFormat);
        setting.beginGroup("Params");
        setting.setIniCodec(QTextCodec::codecForName("GBK"));
        //page1 station
        ui->lineEdit_stationname->setText(QString::fromUtf8(setting.value("StationName").toString().toLocal8Bit()));
        ui->lineEdit_path->setText(QString::fromUtf8(setting.value("DataPath").toString().toLocal8Bit()));
        ui->spinBox_LocalPort->setValue(setting.value("LocalPort").toInt());
        //page2 trigger
        ui->radioButton_singletrigger->setChecked(setting.value("Trigger").toString() == "Single");
        ui->radioButton_continuetrigger->setChecked(setting.value("Trigger").toString() == "Continue");
        ui->spinBox_triggerinterval->setValue(setting.value("TriggerInterval").toInt());
        ui->spinBox_searchLimt->setValue(setting.value("SearchTimeLimit").toInt());
        //page3 pump control
        ui->comboBox_com->setCurrentText(setting.value("Pumpcontrol_Com").toString());
        ui->comboBox_data->setCurrentText(setting.value("Pumpcontrol_Data").toString());
        ui->comboBox_parity->setCurrentText(setting.value("Pumpcontrol_Parity").toString());
        ui->comboBox_portrate->setCurrentText(setting.value("Pumpcontrol_PortRate").toString());
        ui->comboBox_stop->setCurrentText(setting.value("Pumpcontrol_Stop").toString());
        //page4 external control
        ui->radioButton_net->setChecked(setting.value("External").toString() == "Net");
        ui->radioButton_serial->setChecked(setting.value("External").toString() == "Serial");
        ui->comboBox_com_1->setCurrentText(setting.value("External_Com").toString());
        ui->comboBox_data_1->setCurrentText(setting.value("External_Data").toString());
        ui->comboBox_parity_1->setCurrentText(setting.value("External_Parity").toString());
        ui->comboBox_portrate_1->setCurrentText(setting.value("External_PortRate").toString());
        ui->comboBox_stop_1->setCurrentText(setting.value("External_Stop").toString());
        ui->spinBox_port->setValue(setting.value("External_NetPort").toInt());
        //page5 server
        ui->lineEdit__severip->setText(setting.value("ServerIP").toString());
        ui->spinBox_serverport->setValue(setting.value("ServerPort").toInt());
        //page6 calib
        ui->spinBox_EM->setValue(setting.value("EM").toInt());
        ui->spinBox_EX->setValue(setting.value("EX").toInt());
        ui->doubleSpinBox_peak->setValue(setting.value("Peak").toFloat());
        ui->doubleSpinBox_density->setValue(setting.value("Density").toFloat());
        setting.endGroup();
    }
}

void ConfigDialog::saveConfig()
{
    QSettings setting("ConfigFiles/Config.ini",QSettings::IniFormat);
    setting.beginGroup("Params");
    setting.setIniCodec(QTextCodec::codecForName("GBK"));
    //page 1 station
    setting.setValue("StationName",QString::fromLocal8Bit(ui->lineEdit_stationname->text().toUtf8()));
    setting.setValue("DataPath",QString::fromLocal8Bit(ui->lineEdit_path->text().toUtf8()));
    setting.setValue("LocalPort",ui->spinBox_LocalPort->value());
    //page 2 trigger
    setting.setValue("Trigger",ui->radioButton_singletrigger->isChecked()?"Single":"Continue");
    setting.setValue("TriggerInterval",ui->spinBox_triggerinterval->value());
    setting.setValue("SearchTimeLimit",ui->spinBox_searchLimt->value());
    //page 3 pump control
    setting.setValue("Pumpcontrol_Com",ui->comboBox_com->currentText());
    setting.setValue("Pumpcontrol_Data",ui->comboBox_data->currentText());
    setting.setValue("Pumpcontrol_Parity",ui->comboBox_parity->currentText());
    setting.setValue("Pumpcontrol_PortRate",ui->comboBox_portrate->currentText());
    setting.setValue("Pumpcontrol_Stop",ui->comboBox_stop->currentText());
    //page 4 external control
    setting.setValue("External",ui->radioButton_net->isChecked()?"Net":"Serial");
    setting.setValue("External_Com",ui->comboBox_com_1->currentText());
    setting.setValue("External_Data",ui->comboBox_data_1->currentText());
    setting.setValue("External_Parity",ui->comboBox_parity_1->currentText());
    setting.setValue("External_PortRate",ui->comboBox_portrate_1->currentText());
    setting.setValue("External_Stop",ui->comboBox_stop_1->currentText());
    setting.setValue("External_NetPort",ui->spinBox_port->value());
    //page 5 server
    setting.setValue("ServerIP",ui->lineEdit__severip->text());
    setting.setValue("ServerPort",ui->spinBox_serverport->value());
    //page 6 calib
    setting.setValue("EX",ui->spinBox_EX->value());
    setting.setValue("EM",ui->spinBox_EM->value());
    setting.setValue("Peak",ui->doubleSpinBox_peak->value());
    setting.setValue("Density",ui->doubleSpinBox_density->value());
    setting.endGroup();
}
void ConfigDialog::saveAndExit()
{
    saveConfig();
    close();
}
void ConfigDialog::listSerialPort()
{
    ui->comboBox_com->clear();
    ui->comboBox_com_1->clear();

    QList<QSerialPortInfo>  list = QSerialPortInfo::availablePorts();
    for(int i=0; i<list.size(); i++)
    {
        ui->comboBox_com->addItem(list.at(i).portName());
        ui->comboBox_com_1->addItem(list.at(i).portName());
    }
}
void ConfigDialog::chooseFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this,tr("选择数据所在文件夹"),tr(""));
    if(dir.size())
    {
        ui->lineEdit_path->setText(dir);
    }
}

#include "pumpcontroldlg.h"
#include "ui_pumpcontroldlg.h"
#include <QDebug>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QSerialPort>
#include <QModbusRtuSerialMaster>
#include <QModbusReply>
#include <QSettings>
#include <QTime>
#include <QString>
#include "sleepThread.h"
#include <QFile>
#include <QTextStream>

PumpControlDlg::PumpControlDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PumpControlDlg)
{
    ui->setupUi(this);
    setWindowProperty(0);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);  //Qt::WindowMinimizeButtonHint
    setWindowModality(Qt::NonModal);
    setWindowIcon(QIcon(QPixmap("://icons//tube.png")));
    autoFlag = false;
    autoHide = false;
    listPort();
    setDefaultValue();
    loadConfig();
    connect(ui->pushButton_load,SIGNAL(clicked(bool)),this,SLOT(loadConfig()));
    connect(ui->pushButton_open,SIGNAL(clicked(bool)),this,SLOT(openPort()));
    connect(ui->pushButton_save,SIGNAL(clicked(bool)),this,SLOT(saveConfig()));
    connect(ui->pushButton_readao,SIGNAL(clicked(bool)),this,SLOT(readAO()));
    connect(ui->pushButton_readrelay,SIGNAL(clicked(bool)),this,SLOT(readRelay()));
    connect(ui->pushButton_setrelay,SIGNAL(clicked(bool)),this,SLOT(setRelay()));
    connect(ui->pushButton_setao,SIGNAL(clicked(bool)),this,SLOT(setAO()));
    connect(ui->pushButton_start,SIGNAL(clicked(bool)),this,SLOT(startTest()));
    connect(ui->pushButton_stop,SIGNAL(clicked(bool)),this,SLOT(haltTest()));
    connect(ui->pushButton_wash,SIGNAL(clicked(bool)),this,SLOT(startStepTwo()));

    timer = new QTimer(this);
    timerCopy = new QTimer(this);
    waittimer = new QTimer(this);
    setBtnEnable(false);

    startTimer = new QTimer(this);
    startTimer->setInterval(2000);
    connect(startTimer,SIGNAL(timeout()),this,SLOT(checkStartFlag()));

    thread = new SerialThread;
    connect(thread,SIGNAL(portState(bool)),this,SLOT(setOpenState(bool)));
    connect(thread,SIGNAL(finishFeedback(CmdType,QModbusDataUnit)),this,SLOT(getFeedback(CmdType,QModbusDataUnit)));

//    thread->start();
//    checkStartFlag();
/*
    if(autoFlag)
    {
        bool ret = openPort();
        if(ret)
        {
            if(ui->pushButton_open->isEnabled())
            {
                QTimer::singleShot(2000,this,SLOT(startTest()));
                if(autoHide)
                {
                    hide();
                }
            }
        }
    }
    */
}

PumpControlDlg::~PumpControlDlg()
{
    mCmdList.clear();
    thread->closePort();
    delete thread;
    delete ui;
}

void PumpControlDlg::eventLog(QString string)
{
    QFile file("operationlog.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\t" << string << "\n";
}

void PumpControlDlg::keyPressEvent(QKeyEvent *e)
{
    if((e->key() == Qt::Key_Q)  && (e->modifiers().testFlag(Qt::ControlModifier)))
    {
        setWindowProperty(0);
    }
    else if((e->key() == Qt::Key_W)  && (e->modifiers().testFlag(Qt::ControlModifier)))
    {
        setWindowProperty(1);
    }
    else
    {

    }
}
void PumpControlDlg::setWindowProperty(bool b)
{
    if(b)
    {
        setFixedSize(880,427);
    }
    else
    {
        setFixedSize(570,427);
    }
}
void PumpControlDlg::listPort()
{
    ui->comboBox->clear();
    QList<QSerialPortInfo>  list = QSerialPortInfo::availablePorts();
    for(int i=0; i<list.size(); i++)
    {
        ui->comboBox->addItem(list.at(i).portName());
    }
}
bool PumpControlDlg::openPort()
{
    thread->setPort(ui->comboBox->currentText());
    bool ret = thread->openPort();
    return ret;
}
void PumpControlDlg::saveConfig()
{
    QSettings setting("ConfigFiles/PumpControl.ini",QSettings::IniFormat);
    setting.beginGroup("Params");
    setting.setValue("Com",ui->comboBox->currentText());
    setting.setValue("speed1",ui->doubleSpinBox_s1->value());
    setting.setValue("speed2",ui->doubleSpinBox_s2->value());
    setting.setValue("speed3",ui->doubleSpinBox_s3->value());
    setting.setValue("speed4",ui->doubleSpinBox_s4->value());
    setting.setValue("timeA",ui->spinBox_A->value());
    setting.setValue("timeB",ui->spinBox_B->value());
    setting.setValue("timeC",ui->spinBox_C->value());
    setting.setValue("timeD",ui->spinBox_D->value());
    setting.setValue("timeE",ui->spinBox_E->value());
    setting.setValue("timeF",ui->spinBox_F->value());
    setting.setValue("timeG",ui->spinBox_G->value());
    setting.setValue("timeH",ui->spinBox_H->value());
    setting.setValue("timeI",ui->spinBox_I->value());
    setting.setValue("timeJ",ui->spinBox_J->value());
    setting.setValue("timeK",ui->spinBox_K->value());
    setting.setValue("timeL",ui->spinBox_L->value());
    setting.setValue("timeM",ui->spinBox_M->value());
    setting.endGroup();
    QMessageBox::information(this,tr("提示"),tr("保存完毕！"));
}
void PumpControlDlg::loadConfig()
{
    if(QFile::exists("ConfigFiles/PumpControl.ini"))
    {
        QSettings setting("ConfigFiles/PumpControl.ini",QSettings::IniFormat);
        setting.beginGroup("Params");
        autoHide = setting.value("autohide").toBool();
        ui->comboBox->setCurrentIndex(ui->comboBox->findText(setting.value("Com").toString()));
        ui->doubleSpinBox_s1->setValue(setting.value("speed1").toDouble());
        ui->doubleSpinBox_s2->setValue(setting.value("speed2").toDouble());
        ui->doubleSpinBox_s3->setValue(setting.value("speed3").toDouble());
        ui->doubleSpinBox_s4->setValue(setting.value("speed4").toDouble());

        ui->spinBox_A->setValue(setting.value("timeA").toInt());
        ui->spinBox_B->setValue(setting.value("timeB").toInt());
        ui->spinBox_C->setValue(setting.value("timeC").toInt());
        ui->spinBox_D->setValue(setting.value("timeD").toInt());
        ui->spinBox_E->setValue(setting.value("timeE").toInt());
        ui->spinBox_F->setValue(setting.value("timeF").toInt());
        ui->spinBox_G->setValue(setting.value("timeG").toInt());
        ui->spinBox_H->setValue(setting.value("timeH").toInt());
        ui->spinBox_I->setValue(setting.value("timeI").toInt());
        ui->spinBox_J->setValue(setting.value("timeJ").toInt());
        ui->spinBox_K->setValue(setting.value("timeK").toInt());
        ui->spinBox_L->setValue(setting.value("timeL").toInt());
        ui->spinBox_M->setValue(setting.value("timeM").toInt());
        setting.endGroup();
    }
}
void PumpControlDlg::startTest()
{
    QFile file("pumpcontrol.ini");
    file.open(QIODevice::ReadWrite);
    QString content = "Start=0";
    file.write(content.toLatin1());
    file.close();

    startTimer->stop();

    ui->pushButton_save->setDisabled(true);
    ui->pushButton_start->setDisabled(true);
    ui->pushButton_load->setDisabled(true);
    ui->pushButton_open->setDisabled(true);
    ui->pushButton_readao->setDisabled(true);
    ui->pushButton_readrelay->setDisabled(true);
    ui->pushButton_setao->setDisabled(true);
    ui->pushButton_setrelay->setDisabled(true);
    ui->pushButton_wash->setDisabled(true);
//    setWindowFlags(Qt::WindowMinimizeButtonHint);
//    show();

    thread->setStop(true);
    stop_flag = false;
    ui->plainTextEdit->clear();
//    Step1();
    bulidCMDOne();
//    thread->appenCMD(mCmdList.at(0));
    thread->sendCMD(mCmdList.at(0));
}

void PumpControlDlg::readAO()
{
    QVector<quint16> value;
    value.append(4);
    QMap<CmdType,QVector<quint16>> map;
    map.insert(ReadAO,value);
    mCmdList.clear();
    mCmdList.insert(0,map);
    thread->sendCMD(mCmdList.at(0));
}

void PumpControlDlg::readRelay()
{
    QVector<quint16> value;
    value.append(8);
    QMap<CmdType,QVector<quint16>> map;
    map.insert(ReadRelay,value);
    mCmdList.clear();
    mCmdList.insert(0,map);
    thread->sendCMD(mCmdList.at(0));
}

void PumpControlDlg::setRelay()
{
    QVector<quint16> value;
    value.append((quint16)ui->checkBox->isChecked());
    value.append((quint16)ui->checkBox_2->isChecked());
    value.append((quint16)ui->checkBox_3->isChecked());
    value.append((quint16)ui->checkBox_4->isChecked());
    value.append((quint16)ui->checkBox_5->isChecked());
    value.append((quint16)ui->checkBox_6->isChecked());
    value.append((quint16)ui->checkBox_7->isChecked());
    value.append((quint16)ui->checkBox_8->isChecked());

    QMap<CmdType,QVector<quint16>> map;
    map.insert(SetRelay,value);
    mCmdList.clear();
    mCmdList.insert(0,map);
    thread->sendCMD(mCmdList.at(0));
}


void PumpControlDlg::setAO()
{
    QVector<quint16> value;
    value.append(voltageToDigital(ui->doubleSpinBox_speed1->value()));
    value.append(voltageToDigital(ui->doubleSpinBox_speed2->value()));
    value.append(voltageToDigital(ui->doubleSpinBox_speed3->value()));
    value.append(voltageToDigital(ui->doubleSpinBox_speed4->value()));
    value.insert(0,4);
    QMap<CmdType,QVector<quint16>> map;
    map.insert(SetAO,value);
    mCmdList.clear();
    mCmdList.insert(0,map);
    thread->sendCMD(mCmdList.at(0));
}


void PumpControlDlg::setDefaultValue()
{
    if(!QFile::exists("ConfigFiles/PumpControl.ini"))
    {
        QFile file("ConfigFiles/PumpControl.ini");
        file.open(QIODevice::ReadWrite);
        file.close();
        QSettings setting("ConfigFiles/PumpControl.ini",QSettings::IniFormat);
        setting.setValue("autohide",0);
        setting.setValue("Com","Com1");
        setting.setValue("speed1",10);
        setting.setValue("speed2",10);
        setting.setValue("speed3",10);
        setting.setValue("speed4",10);
        setting.setValue("timeA",5);
        setting.setValue("timeB",10);
        setting.setValue("timeC",5);
        setting.setValue("timeD",10);
        setting.setValue("timeE",5);
        setting.setValue("timeF",10);
        setting.setValue("timeG",5);
        setting.setValue("timeH",10);
        setting.setValue("timeI",5);
        setting.setValue("timeJ",15);
        setting.setValue("timeK",5);
        setting.setValue("timeL",15);
        setting.setValue("timeL",5);
    }
}
int PumpControlDlg::voltageToDigital(double value)
{
    int digital = int(value*204.75 + 2047.5);
    return digital;
}
double PumpControlDlg::digitalToVoltage(int value)
{
    double voltage = 0.004884 * (double)value - 10.0;;
    return voltage;
}
void PumpControlDlg::setPump(int index, bool b_enable)
{
    QVector<quint16> value;
    if(b_enable)
    {
        switch (index) {
        case 1:
            value.append(voltageToDigital(ui->doubleSpinBox_s1->value()));
            break;
        case 2:
            value.append(voltageToDigital(ui->doubleSpinBox_s2->value()));
            break;
        case 3:
            value.append(voltageToDigital(ui->doubleSpinBox_s3->value()));
            break;
        case 4:
            value.append(voltageToDigital(ui->doubleSpinBox_s4->value()));
            break;
        default:
            break;
        }
    }
    else
    {
        value.append(voltageToDigital(0));
    }
    value.insert(0,index);
    QMap<CmdType,QVector<quint16>> map;
    map.insert(SetAO,value);
    mCmdList.append(map);
}
void PumpControlDlg::setRelay(QString str)
{
    QVector<quint16> value;
    for(int i=0;i<str.size();i++)
    {
        value.append((quint16)(str.at(str.size() - 1 - i) == '1'));
    }
    QMap<CmdType,QVector<quint16>> map;
    map.insert(SetRelay,value);
    mCmdList.append(map);
}

void PumpControlDlg::appendOperation(QString str)
{
    ui->plainTextEdit->appendPlainText(QTime::currentTime().toString("hh:mm:ss") + "\t" + str);
    eventLog(str);
}
void PumpControlDlg::endTest()
{
    appendOperation("End Test");

    setExperimentFinish();

    ui->pushButton_save->setDisabled(false);
    ui->pushButton_start->setDisabled(false);
    ui->pushButton_load->setDisabled(false);
    ui->pushButton_open->setDisabled(false);
    ui->pushButton_readao->setDisabled(false);
    ui->pushButton_readrelay->setDisabled(false);
    ui->pushButton_setao->setDisabled(false);
    ui->pushButton_setrelay->setDisabled(false);
    ui->pushButton_wash->setDisabled(false);
//    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
//    show();
/*
    if(autoFlag)
    {
        QTimer::singleShot(2000,this,SLOT(close()));
    }
    */
//    startTimer->start();
    emit washFinished();
}
void PumpControlDlg::setStepOneFinish()
{
    QFile file("flag.inf");
    file.open(QIODevice::ReadWrite);

    QString content = "StepFlagOne=1\r\nStepFlagTwo=0\r\nFinish=0";
    file.write(content.toLatin1());
    file.close();
}
void PumpControlDlg::setExperimentFinish()
{
    QFile file("flag.inf");
    file.open(QIODevice::ReadWrite);

    QString content = "StepFlagOne=0\r\nStepFlagTwo=0\r\nFinish=1";
    file.write(content.toLatin1());
    file.close();
}
void PumpControlDlg::checkRestart()
{
    readFlag();
//    QMessageBox::information(this,"info",QString::number(map.value("StepFlagTwo")));
    if(map.value("StepFlagTwo"))
    {
          appendOperation("check restart finish");
          stopTimer();
//        drain_OutputF7000();
          bulidCMDTwo();
          thread->sendCMD(mCmdList.at(0));

    };
    SleeperThread::msleep(50);
}
void PumpControlDlg::startStepTwo()
{
    bulidCMDTwo();
    thread->sendCMD(mCmdList.at(0));
    ui->pushButton_wash->setDisabled(true);
    ui->pushButton_start->setDisabled(true);
}
void PumpControlDlg::readFlag()
{
    QFile file("flag.inf");
    file.open(QIODevice::ReadWrite);

    QString content = file.readAll();
    file.close();

    QStringList list;
    list << "StepFlagOne" << "StepFlagTwo" << "Finish";
    map.clear();
    for(int i = 0; i < list.size(); i++)
    {
        map[list.at(i)] = content.mid(content.indexOf(list.at(i),0,Qt::CaseInsensitive) + list.at(i).size() + 1,1).toInt();
    }
}
void PumpControlDlg::sendTestSignal()
{
    appendOperation("sendTestSignal");
    setStepOneFinish();
/*
    timer->setInterval(2000);
    connect(timer,SIGNAL(timeout()),this,SLOT(checkRestart()));
    timer->start();
    */
    setRelay("01000000");
    emit firstStepFinished();
}

void PumpControlDlg::haltTest()
{
    mCmdList.clear();
    thread->setStop(true);
    stop_flag = false;
    timer->stop();
    waittimer->stop();

    setPump(1,false);
    setPump(2,false);
    setPump(3,false);
    setPump(4,false);

    setRelay("01000000");
    buildLogCMD(/*Step*/19);
    thread->sendCMD(mCmdList.at(0));
}

void PumpControlDlg::Step1()
{

    setRelay("11000100"); //
    setPump(3,true);
    setPump(1,true);

    if(ui->spinBox_A->value() > ui->spinBox_M->value())
    {
        timer->setInterval(ui->spinBox_A->value()*1000);
        timerCopy->setInterval(ui->spinBox_M->value()*1000);
    }
    else
    {
        timer->setInterval(ui->spinBox_M->value()*1000);
        timerCopy->setInterval(ui->spinBox_A->value()*1000);
    }

    connect(timer,SIGNAL(timeout()),this,SLOT(Step2()));
    connect(timerCopy,SIGNAL(timeout()),this,SLOT(stopStep1()));

    timer->start();
    timerCopy->start();

    appendOperation("Step1");
}
void PumpControlDlg::stopStep1()
{
    if(ui->spinBox_A->value() > ui->spinBox_M->value())
    {
        setPump(1,false);
        setRelay("01000100"); //
    }
    else
    {
        setPump(3,false);
    }

    disconnect(timerCopy,SIGNAL(timeout()),this,SLOT(stopStep1()));
    timerCopy->stop();
}
void PumpControlDlg::Step2()
{
    stopTimer();

    if(ui->spinBox_A->value() > ui->spinBox_M->value())
    {
        setPump(3,false);
    }
    else
    {
        setPump(1,false);
    }

    setRelay("01000000");
    setPump(4,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step3()));
    startTimerCustom(ui->spinBox_B->value(),"Step2");
}
void PumpControlDlg::Step3()
{
    stopTimer();

    setPump(4,false);
    setRelay("01000001");
    setPump(1,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step4()));
    startTimerCustom(ui->spinBox_C->value()*2,"Step3");
}
void PumpControlDlg::Step4()
{
    stopTimer();

    setPump(1,false);
    setRelay("01000000");
    setPump(4,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step5A()));
    startTimerCustom(ui->spinBox_D->value(),"Step4");
}
void PumpControlDlg::Step5A()
{
    stopTimer();

    setPump(4,false);
    setRelay("01000001");
    setPump(1,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step5B()));
    startTimerCustom(ui->spinBox_C->value(),"Step5A");
}
void PumpControlDlg::Step5B()
{
    stopTimer();

    setRelay("11000000");
    setPump(1,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step6()));
    startTimerCustom(ui->spinBox_C->value(),"Step5B");
}
void PumpControlDlg::Step6()
{
    stopTimer();

    setPump(1,false);
    setRelay("01000000");
    setPump(4,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step7A()));
    startTimerCustom(ui->spinBox_D->value(),"Step6");
}
void PumpControlDlg::Step7A()
{
    stopTimer();

    setPump(4,false);
    setRelay("11000000");
    setPump(1,true);
    setPump(2,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step7B()));
    startTimerCustom(ui->spinBox_E->value(),"Step7A");
}
void PumpControlDlg::Step7B()
{
    stopTimer();

    setRelay("01000001");

    connect(timer,SIGNAL(timeout()),this,SLOT(Step8()));
    startTimerCustom(ui->spinBox_F->value(),"Step7B");
}
void PumpControlDlg::Step8()
{
    stopTimer();

    setPump(1,false);
    setRelay("01000000");

    connect(timer,SIGNAL(timeout()),this,SLOT(Step9()));
    startTimerCustom(ui->spinBox_G->value(),"Step8");
}
void PumpControlDlg::Step9()
{
    stopTimer();

    setPump(2,false);
    setPump(3,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(sendTestSignal()));
    startTimerCustom(ui->spinBox_H->value(),"Step9");
}
void PumpControlDlg::Step10()
{
 // check restart
}
void PumpControlDlg::Step11()
{
    setRelay("01000100");
    setPump(3,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step12()));
    startTimerCustom(ui->spinBox_I->value(),"Step11");
}
void PumpControlDlg::Step12()
{
    stopTimer();

    setPump(3,false);
    setRelay("01000000");
    setPump(4,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step13()));
    startTimerCustom(ui->spinBox_J->value(),"Step12");
}
void PumpControlDlg::Step13()
{
    stopTimer();

    setPump(4,false);
    setRelay("01000001");
    setPump(1,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step14()));
    startTimerCustom(ui->spinBox_K->value(),"Step13");
}
void PumpControlDlg::Step14()
{
    stopTimer();

    setPump(1,false);
    setRelay("01000000");
    setPump(4,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step15()));
    startTimerCustom(ui->spinBox_L->value(),"Step14");
}
void PumpControlDlg::Step15()
{
    stopTimer();

    setPump(4,false);
    setRelay("01000001");
    setPump(1,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step16()));
    startTimerCustom(ui->spinBox_K->value(),"Step15");
}
void PumpControlDlg::Step16()
{
    stopTimer();

    setPump(1,false);
    setRelay("01000000");
    setPump(3,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(Step17()));
    startTimerCustom(ui->spinBox_H->value(),"Step16");
}
void PumpControlDlg::Step17()
{
    stopTimer();

    setPump(3,false);
    setPump(4,true);

    connect(timer,SIGNAL(timeout()),this,SLOT(endTest()));
    startTimerCustom(ui->spinBox_L->value(),"Step17");
}
void PumpControlDlg::stopTimer()
{
    timer->disconnect();
    timer->stop();
}
void PumpControlDlg::startTimerCustom(int value, QString str)
{
    timer->setInterval(value*1000);
    timer->start();
    appendOperation(str);
}
void PumpControlDlg::testcase()
{
    setPump(1,true);
    setPump(2,true);
    setPump(3,true);
    setPump(4,true);

    setRelay("10101010");
}
void PumpControlDlg::setBtnEnable(bool b)
{
    ui->pushButton_start->setEnabled(b);
    ui->pushButton_stop->setEnabled(b);
    ui->pushButton_readao->setEnabled(b);
    ui->pushButton_readrelay->setEnabled(b);
    ui->pushButton_setao->setEnabled(b);
    ui->pushButton_setrelay->setEnabled(b);
    ui->pushButton_wash->setEnabled(b);
}
void PumpControlDlg::getTimeElapse()
{
//    ui->spinBox_Time->setValue(mSendTime.msecsTo(QTime::currentTime()));
}
void PumpControlDlg::errorLog(QString string)
{
    QFile file("errorlog.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\t" << string << "\n";
}
void PumpControlDlg::setOpenState(bool b)
{
    if(b)
    {
        ui->pushButton_open->setText(tr("关闭"));
        setBtnEnable(true);
    }
    else
    {
        ui->pushButton_open->setText(tr("打开"));
        setBtnEnable(false);
        mCmdList.clear();
    }

}
void PumpControlDlg::getFeedback(CmdType ctype, QModbusDataUnit result)
{
//    qDebug() << "FeedBack : " <<ctype ;
    if(ctype == ReadAO)
    {
        ui->doubleSpinBox_speed1->setValue(digitalToVoltage(result.value(0)));
        ui->doubleSpinBox_speed2->setValue(digitalToVoltage(result.value(1)));
        ui->doubleSpinBox_speed3->setValue(digitalToVoltage(result.value(2)));
        ui->doubleSpinBox_speed4->setValue(digitalToVoltage(result.value(3)));
        qDebug() << result.value(0) << result.value(1) << result.value(2) << result.value(3);
    }
    else if(ctype == ReadRelay)
    {
        ui->checkBox->setChecked(result.value(0) == 1);
        ui->checkBox_2->setChecked(result.value(1) == 1);
        ui->checkBox_3->setChecked(result.value(2) == 1);
        ui->checkBox_4->setChecked(result.value(3) == 1);
        ui->checkBox_5->setChecked(result.value(4) == 1);
        ui->checkBox_6->setChecked(result.value(5) == 1);
        ui->checkBox_7->setChecked(result.value(6) == 1);
        ui->checkBox_8->setChecked(result.value(7) == 1);
    }
    else if(ctype == Log)
    {
        if(result.value(0) == 10)
        {
            sendTestSignal();
        }
        else if(result.value(0) == 18)
        {
            endTest();
        }
        else if(result.value(0) == 19)
        {
            ui->pushButton_save->setDisabled(false);
            ui->pushButton_start->setDisabled(false);
            ui->pushButton_load->setDisabled(false);
            ui->pushButton_open->setDisabled(false);
            ui->pushButton_readao->setDisabled(false);
            ui->pushButton_readrelay->setDisabled(false);
            ui->pushButton_setao->setDisabled(false);
            ui->pushButton_setrelay->setDisabled(false);
            ui->pushButton_wash->setDisabled(false);
//            setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
//            show();
        }
        else
        {
            appendOperation(QString("Step%1").arg(result.value(0)));

        }
        autosendCMD();
    }
    else if(ctype == Wait)
    {
//        int interval = result.value(0);
//        qDebug() << "interval : " << interval;
//        waittimer->setInterval(interval*1000);
//        connect(waittimer,SIGNAL(timeout()),this,SLOT(autosendCMD()));
//        waittimer->start();
        autosendCMD();
    }
    else if(ctype == SetAO)
    {
        autosendCMD();
    }
    else if(ctype == SetRelay)
    {
        autosendCMD();
    }
    else
    {
 //       autosendCMD();
    }

}
void PumpControlDlg::bulidCMDOne()
{
    mCmdList.clear();

    buildLogCMD(/*Step*/1);
    setRelay("11000100");
    setPump(3,true);
    setPump(1,true);
    if(ui->spinBox_A->value() > ui->spinBox_M->value())
    {
        buildWaitCMD(ui->spinBox_M->value());
        setPump(1,false);
        setRelay("01000100");
        buildWaitCMD(ui->spinBox_A->value() - ui->spinBox_M->value());
        setPump(3,false);
    }
    else
    {
        buildWaitCMD(ui->spinBox_A->value());
        setPump(3,false);
        buildWaitCMD(ui->spinBox_M->value() - ui->spinBox_A->value());
        setPump(1,false);
        setRelay("01000100");
    }

    buildLogCMD(/*Step*/2);
    setRelay("01000000");
    setPump(4,true);
    buildWaitCMD(ui->spinBox_B->value());
    setPump(4,false);

    buildLogCMD(/*Step*/3);
    setRelay("01000001");
    setPump(1,true);
    buildWaitCMD(ui->spinBox_C->value()*2);
    setPump(1,false);

    buildLogCMD(/*Step*/4);
    setRelay("01000000");
    setPump(4,true);
    buildWaitCMD(ui->spinBox_D->value());
    setPump(4,false);

    buildLogCMD(/*Step*/5);
    setRelay("01000001");
    setPump(1,true);
    buildWaitCMD(ui->spinBox_C->value());
    setRelay("11000000");
    buildWaitCMD(ui->spinBox_C->value());
    setPump(1,false);

    buildLogCMD(/*Step*/6);
    setRelay("01000000");
    setPump(4,true);
    buildWaitCMD(ui->spinBox_D->value());
    setPump(4,false);

    buildLogCMD(/*Step*/7);
    setRelay("11000000");
    setPump(1,true);
    setPump(2,true);
    buildWaitCMD(ui->spinBox_E->value());
    setRelay("01000001");
    buildWaitCMD(ui->spinBox_F->value());
    setPump(1,false);

    buildLogCMD(/*Step*/8);
    setRelay("01000000");
    buildWaitCMD(ui->spinBox_G->value());
    setPump(2,false);

    buildLogCMD(/*Step*/9);
    setPump(3,true);
    buildWaitCMD(ui->spinBox_H->value());
    setPump(3,false);

    buildLogCMD(/*Step SendTestSignal*/10);

}
void PumpControlDlg::bulidCMDTwo()
{
    mCmdList.clear();

    buildLogCMD(/*Step*/11);
    setRelay("01000100");
    setPump(3,true);
    buildWaitCMD(ui->spinBox_I->value());
    setPump(3,false);

    buildLogCMD(/*Step*/12);
    setRelay("01000000");
    setPump(4,true);
    buildWaitCMD(ui->spinBox_J->value());
    setPump(4,false);

    buildLogCMD(/*Step*/13);
    setRelay("01000001");
    setPump(1,true);
    buildWaitCMD(ui->spinBox_K->value());
    setPump(1,false);

    buildLogCMD(/*Step*/14);
    setRelay("01000000");
    setPump(4,true);
    buildWaitCMD(ui->spinBox_L->value());
    setPump(4,false);

    buildLogCMD(/*Step*/15);
    setRelay("01000001");
    setPump(1,true);
    buildWaitCMD(ui->spinBox_K->value());
    setPump(1,false);

    buildLogCMD(/*Step*/16);
    setRelay("01000000");
    setPump(3,true);
    buildWaitCMD(ui->spinBox_H->value());
    setPump(3,false);

    buildLogCMD(/*Step*/17);
    setPump(4,true);
    buildWaitCMD(ui->spinBox_L->value());
    setPump(4,false);

    buildLogCMD(/*Step End Test*/18);

}
void PumpControlDlg::autosendCMD()
{
    waittimer->stop();
    if(!mCmdList.isEmpty())
    {
       mCmdList.pop_front();
    }
    if(!mCmdList.isEmpty())
    {
        thread->sendCMD(mCmdList.at(0));
    }

}
void PumpControlDlg::buildLogCMD(int id)
{
    QVector<quint16> value;
    value.append(id);
    QMap<CmdType,QVector<quint16>> map;
    map.insert(Log,value);
    mCmdList.append(map);
}
void PumpControlDlg::buildWaitCMD(int time)
{
    QVector<quint16> value;
    value.append(time);
    QMap<CmdType,QVector<quint16>> map;
    map.insert(Wait,value);
    mCmdList.append(map);
}
void PumpControlDlg::checkStartFlag()
{
    QFile file("pumpcontrol.ini");
    file.open(QIODevice::ReadWrite);

    QString content = file.readAll();
    file.close();

    QStringList list = content.split('=');

    if(list.at(1) == "1")
    {
        startTimer->stop();
        startTest();
    }
    else
    {

    }
}
void PumpControlDlg::closeEvent(QCloseEvent *event)
{
    event->ignore();
    hide();
}
void PumpControlDlg::start()
{
    qDebug() << "pumpcontrol start";
    if(ui->pushButton_open->text() == tr("打开"))
    {
       openPort();
       SleeperThread::msleep(200);
    }

    if(ui->pushButton_open->text() == tr("关闭"))
    {
        startTest();
    }
}

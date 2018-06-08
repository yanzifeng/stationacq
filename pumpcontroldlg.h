#ifndef PUMPCONTROLDLG_H
#define PUMPCONTROLDLG_H

#include <QDialog>
#include <QModbusClient>
#include <QModbusDataUnit>
#include <QTimer>
#include <QTime>
#include <QMutex>
#include "def.h"
#include "serialthread.h"

namespace Ui {
class PumpControlDlg;
}

class PumpControlDlg : public QDialog
{
    Q_OBJECT

public:
    explicit PumpControlDlg(QWidget *parent = 0);
    ~PumpControlDlg();
protected:
    void keyPressEvent(QKeyEvent *e);
    void closeEvent(QCloseEvent *event);
private:
    void setWindowProperty(bool b);
    void setDefaultValue();
    void setExperimentFinish();
    void setStepOneFinish();
    void readFlag();
    void setBtnEnable(bool);
private slots:
    void listPort();
    bool openPort();
    void saveConfig();
    void loadConfig();

    void readRelay();
    void setRelay();
    void readAO();
    void setAO();

    //功能流程函数
    void Step1();
    void stopStep1();
    void Step2();
    void Step3();
    void Step4();
    void Step5A();
    void Step5B();
    void Step6();
    void Step7A();
    void Step7B();
    void Step8();
    void Step9();
    void Step10();
    void Step11();
    void Step12();
    void Step13();
    void Step14();
    void Step15();
    void Step16();
    void Step17();
    void startTest();
    void sendTestSignal();
    void endTest();
    void checkRestart();
    void getTimeElapse();
    void testcase();
    void autosendCMD();
    void checkStartFlag();
public slots:
    void setOpenState(bool);
    void getFeedback(CmdType,QModbusDataUnit);
    void start();
    void startStepTwo();
    void haltTest();
signals:
    void firstStepFinished();
    void washFinished();
private:
    int voltageToDigital(double value);
    double digitalToVoltage(int value);
    void appendOperation(QString str);

    void setPump(int index, bool b_enable);
    void setRelay(QString relay);
    void errorLog(QString string);
    void stopTimer();
    void startTimerCustom(int,QString);
    void bulidCMDOne();
    void bulidCMDTwo();

    void buildWaitCMD(int);
    void buildLogCMD(int);
    void eventLog(QString string);
private:
    Ui::PumpControlDlg *ui;
    bool autoFlag,autoHide;
    QTimer *timer,*timerCopy,*waittimer,*startTimer;
    QMap<QString,int> map;
    QTime mSendTime;
    SerialThread *thread;
    QList<QMap<CmdType,QVector<quint16>>> mCmdList;
};

#endif // PUMPCONTROLDLG_H

#ifndef SERIALTHREAD_H
#define SERIALTHREAD_H

#include <QThread>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QStringList>
#include <QSerialPort>
#include <QModbusClient>
#include <QModbusDataUnit>
#include <QModbusRtuSerialMaster>
#include <QMap>
#include <QModbusReply>
#include <QMutex>
#include <QTimer>

const int AOADDR = 1;
const int RelayADDR = 2;
enum CmdType {ReadAO,SetAO,ReadRelay,SetRelay,Wait,Log};

class SerialThread : public QThread
{
    Q_OBJECT

public:
    explicit SerialThread(QObject *parent = 0)
        : QThread(parent)
    {
        modbusDevice = new QModbusRtuSerialMaster;
        modbusDevice->setTimeout(1000);
        modbusDevice->setNumberOfRetries(3);

        waitTimer = new QTimer;
        connect(waitTimer,SIGNAL(timeout()),this,SLOT(sendWaitResult()));
        mtimes = 0;
        eventLog(tr("create thread"));
    }
    ~SerialThread()
    {
        requestInterruption();
        quit();
        wait();
        if (modbusDevice)
        {
            qDebug() << "auto release";
            eventLog(tr("auto release"));
            if(modbusDevice->state() == QModbusDevice::ConnectedState)
            {
                modbusDevice->disconnectDevice();
            }
            msleep(100);
        }
        delete modbusDevice;
    }
protected:
    virtual void run();
private:
    void writeModbus();
    void eventLog(QString string);
signals:
    void portState(bool);
    void finishFeedback(CmdType,QModbusDataUnit);
public:
    void setPort(QString);
    void appenCMD(QMap<CmdType,QVector<quint16>>);
    void setStop(bool);
public slots:
    bool openPort();
    void closePort();
    void sendCMD(QMap<CmdType,QVector<quint16>>);
private slots:
    void readyRead();
    void sendWaitResult();
private:
    QModbusClient *modbusDevice;
    QMap<CmdType,QVector<quint16>> mCurrentCmd;
    QList<QMap<CmdType,QVector<quint16>>> mCmdList;
    QModbusReply *reply;
    bool mStop;
    QTimer *waitTimer;
    int mtimes;
};

#endif

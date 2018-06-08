#include "serialthread.h"
#include "def.h"
#include <QFile>
#include <QTextStream>
void SerialThread::run()
{
    // 是否请求终止
    while (!isInterruptionRequested()){
/*
        if(modbusDevice->state() == QModbusDevice::ConnectedState)
        {
            if(mCmdList.size())
            {
                mCurrentCmd = mCmdList.at(0);
                writeModbus();
                mCmdList.clear();
            }
        }*/
        msleep(100);
    }
}
void SerialThread::setPort(QString portname)
{
    modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
        portname);
}

bool SerialThread::openPort()
{

    if (!modbusDevice)
        return false;
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {

        modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
            QSerialPort::NoParity);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
            QSerialPort::Baud9600);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
            QSerialPort::Data8);
        modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
            QSerialPort::OneStop);

        if (!modbusDevice->connectDevice()) {
            QMessageBox::warning(0,tr("警告"),tr("串口打开失败 : %1").arg(modbusDevice->errorString()));
            emit portState(false);
            return false;

        } else {

            emit portState(true);
            mCmdList.clear();
            eventLog(tr("Open Success!"));
            return true;
        }
    }
    else
    {
        modbusDevice->disconnectDevice();
        emit portState(false);
        mCmdList.clear();
        return true;
    }
}
void SerialThread::appenCMD(QMap<CmdType, QVector<quint16>> cmd)
{
    mCmdList.append(cmd);
}
void SerialThread::writeModbus()
{

    CmdType key = mCurrentCmd.keys().at(0);
    QVector<quint16> list = mCurrentCmd.value(key);
//    qDebug() << "writemodbus : " << key << list;
    if(key == SetAO)
    {
        QModbusReply *reply = modbusDevice->sendWriteRequest(QModbusDataUnit(QModbusDataUnit::HoldingRegisters,4-list.at(0),list.mid(1)),AOADDR);

        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, this, &SerialThread::readyRead);
        }
        else
        {
            delete reply; // broadcast replies return immediately
        }
    }
    else if(key == ReadAO)
    {
        QModbusReply *reply = modbusDevice->sendReadRequest(QModbusDataUnit(QModbusDataUnit::HoldingRegisters,0,list.at(0)),AOADDR);
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &SerialThread::readyRead);
        else
            delete reply; // broadcast replies return immediately
    }
    else if(key == SetRelay)
    {
        QModbusReply *reply = modbusDevice->sendWriteRequest(QModbusDataUnit(QModbusDataUnit::Coils,16,list),RelayADDR);
        if (!reply->isFinished())
        {
            connect(reply, &QModbusReply::finished, this, &SerialThread::readyRead);
        }
        else
        {
            delete reply; // broadcast replies return immediately
        }
    }
    else if(key == ReadRelay)
    {
        QModbusReply *reply = modbusDevice->sendReadRequest(QModbusDataUnit(QModbusDataUnit::Coils,16,list.at(0)),RelayADDR);
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &SerialThread::readyRead);
        else
            delete reply; // broadcast replies return immediately
    }
    else if(key == Wait)
    {
/*
        int times = list.at(0)*20;
        int i = 0;
        while(!stop_flag && i < times)
        {
            i++;
            msleep(50);
        }
        QModbusDataUnit unit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters,0,list.at(0));
        unit.setValue(0,list.at(0));
        emit finishFeedback(key,unit);
        */
        qDebug() << QTime::currentTime() << tr("enter wait timer,seconds : %1").arg(list.at(0));
        if(list.at(0) > 0)
        {
             waitTimer->stop();
             waitTimer->setInterval(list.at(0)*1000);
//             waitTimer->setSingleShot(true);
             waitTimer->start();
        }
        else
        {
            QModbusDataUnit unit;
            emit finishFeedback(key,unit);
        }

    }
    else if(key == Log)
    {
        QModbusDataUnit unit = QModbusDataUnit(QModbusDataUnit::HoldingRegisters,0,list.at(0));
        unit.setValue(0,list.at(0));
        emit finishFeedback(key,unit);
    }
    else
    {

    }
}
void SerialThread::readyRead()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;
    if(reply->error() == QModbusDevice::NoError)
    {

        mtimes = 0;
        CmdType key = mCurrentCmd.keys().at(0);
        if(key < 4)
        {
            emit finishFeedback(key,reply->result());
        }
    }
    else
    {
        eventLog(QString("设置失败Code:%1,失败Str:%2").arg(reply->error()).arg(reply->errorString()));
        /*
        modbusDevice->disconnectDevice();
        msleep(100);
        openPort();
        msleep(100);
        sendCMD(mCurrentCmd);*/
        if(mtimes < 3)
        {
            modbusDevice->disconnectDevice();
            msleep(100);
            openPort();
            msleep(100);
            sendCMD(mCurrentCmd);
            mtimes++;
            eventLog(tr("ResentCMD Type : %1").arg(mCurrentCmd.keys().at(0)));
        }
        else
        {
            CmdType key = mCurrentCmd.keys().at(0);
            QModbusDataUnit unit;
            emit finishFeedback(key,unit);
            mtimes = 0;
        }
    }
    reply->deleteLater();
}
void SerialThread::sendCMD(QMap<CmdType, QVector<quint16>> cmd)
{
    mCurrentCmd = cmd;
    writeModbus();
}
void SerialThread::setStop(bool b)
{
    mStop = b;
    stop_flag = b;
    if(b)
    {
        waitTimer->stop();
    }
}
void SerialThread::sendWaitResult()
{
    waitTimer->stop();
//    qDebug() << QTime::currentTime() << tr("sendWaitResult");
    CmdType key = mCurrentCmd.keys().at(0);
    QModbusDataUnit unit;
    emit finishFeedback(key,unit);
}
void SerialThread::closePort()
{
    eventLog(tr("close port begin"));
    while(modbusDevice->state() == QModbusDevice::ConnectedState)
    {
        modbusDevice->disconnectDevice();
        msleep(100);
        eventLog(tr("close port operation"));
    }
    eventLog(tr("close port finished"));
}
void SerialThread::eventLog(QString string)
{
    QFile file("eventlog.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\t" << string << "\n";
}

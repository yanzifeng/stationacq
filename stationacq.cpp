#include "stationacq.h"
#include "ui_stationacq.h"
#include <QStringList>
#include <QMessageBox>
#include <QHostInfo>
#include "configdialog.h"
#include "logdialog.h"
#include "uploaddialog.h"
#include <QTextCodec>
#include "sleepThread.h"
#include <QFileDialog>
#include <QFileInfo>
#include <iostream>
#include <ctime>

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    char buffer[128];
    int written = GetWindowTextA(hwnd, buffer, 128);

    if (written == 12 && strstr(buffer,"FL Solutions") != NULL)
    {


        char clsName_v[256]; //a buffer of  chars
        GetClassNameA (hwnd, clsName_v, 256); //Specifically using ANSI version of the function

        qDebug() << written << buffer << clsName_v;
        QString str = QString::fromLatin1(clsName_v);
        if(str != "CabinetWClass") //排除打开FL_solutions文件夹
        {
           *(HWND*)lParam = hwnd;
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
    return TRUE;
}

HWND GetFLSOLHwnd()
{
    HWND hWnd = NULL;
    EnumWindows(EnumWindowsProc, (LPARAM)&hWnd);
    return hWnd;
}

StationAcq::StationAcq(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::StationAcq)
{
    ui->setupUi(this);
    createConfigFolder();
    createIndicator();
    initParams();
    setTableHeader();
    setWavefrom();
    createStatusBar();
    createToolBar();
    createSocket();
    updateTestCycle();

    connect(ui->action_TestCase,SIGNAL(triggered(bool)),this,SLOT(testcase_socket()));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(updateWaveformDate(int)));
    connect(ui->action_config,SIGNAL(triggered(bool)),this,SLOT(configDlg()));
    connect(ui->action_upload,SIGNAL(triggered(bool)),this,SLOT(uploadDlg()));
    connect(ui->action_logdlg,SIGNAL(triggered(bool)),this,SLOT(logDlg()));
    connect(ui->action_help,SIGNAL(triggered(bool)),this,SLOT(testSimulateKeyPress()));
    connect(ui->action_acq,SIGNAL(triggered(bool)),this,SLOT(startFlsolProcess()));
    connect(ui->action_pumpcontrol,SIGNAL(triggered(bool)),this,SLOT(openPumpControl()));
    connect(ui->action_start,SIGNAL(triggered(bool)),this,SLOT(setStartMarks()));
    connect(ui->action_stop,SIGNAL(triggered(bool)),this,SLOT(stop()));
    connect(ui->action_calib,SIGNAL(triggered(bool)),this,SLOT(setCalibMarks()));
    pumpcontrol = new PumpControlDlg;
    connect(pumpcontrol,SIGNAL(firstStepFinished()),this,SLOT(startAnalyze()));
    connect(pumpcontrol,SIGNAL(washFinished()),this,SLOT(finish()));
    connect(this,SIGNAL(startwash()),pumpcontrol,SLOT(startStepTwo()));
    connect(ui->action_connectServer,SIGNAL(triggered(bool)),this,SLOT(reConnectToServer()));
    loopTimer = new QTimer(this);
    loopTimer->setInterval(1000);
    connect(loopTimer,SIGNAL(timeout()),this,SLOT(onTestLoopTimer()));
    mainTimer = new QTimer(this);
    mainTimer->setInterval(3000);
    connect(mainTimer,SIGNAL(timeout()),this,SLOT(onMainTimer()));
    connect(ui->tableWidget->verticalScrollBar(),SIGNAL(sliderMoved(int)),this,SLOT(showTableValue(int)));
    QTimer::singleShot(1000,this,SLOT(connectToServer()));
//    testcase();
    openServer();
//    testcase_waveform();
//    testcase_settablevalue();
    /*********test waveform**********/
    /*
    srand((unsigned)time(NULL));
    QTimer *testcaseTimer = new QTimer(this);
    testcaseTimer->setInterval(1000);
    connect(testcaseTimer,SIGNAL(timeout()),this,SLOT(testcase_ontimer()));
    testcaseTimer->start();*/
}

StationAcq::~StationAcq()
{
    if(loopTimer->isActive())
    {
        loopTimer->stop();
    }
    if(mainTimer->isActive())
    {
        mainTimer->stop();
    }
    tcpSocket->abort();
    delete pumpcontrol;
    delete loopTimer;
    delete ui;
}
void StationAcq::setTableHeader()
{
    QStringList env_headers;
    env_headers << tr("序号") << tr("ID") << tr("测试时间") << tr("样品名称") << tr("污染来源") << tr("相似度") << tr("超标") << tr("荧光峰1") << tr("水纹强度1") << tr("荧光峰2") << tr("水纹强度2") << tr("荧光峰3") << tr("水纹强度3");

    ui->tableWidget->setHorizontalHeaderLabels(env_headers);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    QList<int> colWidth;
    colWidth << 50 << 50 << 150 << 150 << 120 << 100 << 50 << 100 << 100 << 100 << 100 << 100 << 100;
    for(int i=0; i<colWidth.size(); i++)
    {
        ui->tableWidget->setColumnWidth(i,colWidth.at(i));
    }

}
void StationAcq::SimulateKeyPress(uint key_code)
{
    QString pstr = getProcessNames();
//    qDebug() << pstr;
    if(pstr.contains("flsol.exe",Qt::CaseInsensitive))
    {
        HWND mywindow = NULL;
        uint keyCode = key_code;
        QString str;

        str = "flsol";
        wchar_t* name = new wchar_t[str.length() + 1];
        str.toWCharArray(name);
        /*
        while(mywindow == NULL)
        {
           mywindow = FindWindow(name,0);
           SleeperThread::msleep(100);
        }*/
        mywindow = GetFLSOLHwnd();
        qDebug() << mywindow;
        ShowWindow(mywindow, SW_SHOW);
        ShowWindow(mywindow, SW_RESTORE);
        SetForegroundWindow(mywindow);

        if(mywindow != NULL)
        {
            PostMessage(mywindow, WM_KEYDOWN, keyCode, 0); // key down
        }
        else
        {
            QMessageBox::information(this,tr("提示"),tr("FindWindow Error!"));
        }
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请打开FL_Solutions程序!"));
    }
}
void StationAcq::testSimulateKeyPress()
{
    SimulateKeyPress(VK_F1); // F1 key
}
void StationAcq::startFlsolProcess()
{
    QString str = QFileDialog::getOpenFileName(this,tr("打开FL-Solution软件"),"","*.exe");
    if(str.size())
    {
        QString pstr = getProcessNames();
        if(!pstr.contains("flsol.exe",Qt::CaseInsensitive))
        {
            WinExec(str.toStdString().c_str(),SW_SHOWNORMAL);
        }
        else
        {
            QMessageBox::information(this,tr("提示"),tr("FL_Solutions程序已打开!"));
        }
    }
}
void StationAcq::setWavefrom()
{
    chartview = new ChartView(ui->frame_3);
    QFont font(tr("微软雅黑"),10);
    hBar = new QScrollBar(this);
    hBar->setOrientation(Qt::Horizontal);
    hBar->setMinimum(1);hBar->setMaximum(1);
    hBar->setValue(1);hBar->setVisible(false);
    connect(hBar,SIGNAL(valueChanged(int)),this,SLOT(updateWaveformWhenScrollBarMove(int)));

    QVBoxLayout *vlayout = new QVBoxLayout(ui->frame_3);
    vlayout->addWidget(chartview);
    vlayout->addWidget(hBar);
    vlayout->setMargin(1);
    vlayout->setContentsMargins(0,0,0,0);
    ui->frame->setContentsMargins(0,0,0,0);
    ui->frame->setStyleSheet("Background-color : lightGray");
    vlayout->setSpacing(0);
    chartview->setRenderHint(QPainter::Antialiasing);
    chartview->setBackgroundBrush(QBrush(Qt::lightGray));

    chart = new QChart();

    chart->legend()->hide();
    chart->createDefaultAxes();
//    chart->setTitle(tr("曲线图"));
    chartview->setChart(chart);
    chart->setTheme(QChart::ChartThemeBlueCerulean);

    seriesTest = new QLineSeries();
    seriesTest->setColor(Qt::blue);
    seriesTest->setPen(QPen(Qt::blue,2));
    seriesTest->setPointsVisible(true);
    chart->addSeries(seriesTest);

    axisX = new QDateTimeAxis;
    axisX->setFormat("yyyy/MM/dd HH:mm");
    axisX->setTitleText("时间");
    axisX->setLabelsFont(font);
    axisX->setTitleVisible(false);
    axisX->setTickCount(6);

    axisY = new QValueAxis;
    axisY->setRange(0, 1);
    axisY->setLabelFormat("%.2f");
    axisY->setTitleText("相似度");
    axisY->setLabelsFont(font);
    axisY->setVisible(true);
    axisY->setTickCount(5);

//    axisY->setLabelsColor(Qt::white);

    chart->setAxisX(axisX,seriesTest);
    chart->setAxisY(axisY,seriesTest);

    textitem = new QGraphicsTextItem(chart);
    textitem->setFont(QFont(tr("微软雅黑"),10));
    chartview->addTextItem(textitem);
    textitem->setZValue(1);
    axisX->setMin(QDateTime::currentDateTime());
    axisX->setMax(QDateTime::currentDateTime().addSecs(60*60*24));
    textitem->hide();
//    connect(seriesTest,SIGNAL(pressed(QPointF)),this,SLOT(showPointValue(QPointF)));
//    connect(seriesTest,SIGNAL(released(QPointF)),this,SLOT(hidePointValue(QPointF)));
    connect(seriesTest,SIGNAL(hovered(QPointF,bool)),this,SLOT(showHidePoint(QPointF,bool)));
}
void StationAcq::testcase_waveform()
{
    srand((unsigned)time(NULL));
    for(int i=0;i<10;i++)
    {
        seriesTest->append(QDateTime::currentDateTime().addSecs(i*60*60).toMSecsSinceEpoch(),((double)rand())/RAND_MAX);
    }
    axisX->setMin(QDateTime::currentDateTime());
    axisX->setMax(QDateTime::currentDateTime().addSecs(60*60*24));
}
void StationAcq::updateWaveform()
{
    int interval;
    switch (ui->comboBox->currentIndex()) {
        case 0:
            interval = 1;  //每日
            break;
        case 1:
            interval = 7;  //每周
            break;
        case 2:
            interval = 30;  //每月
            break;
        default:
            break;
    }
    int rowcount = ui->tableWidget->rowCount();
    seriesTest->append(QDateTime::fromString(ui->tableWidget->item(rowcount-1,2)->text(),"yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch(),ui->tableWidget->item(rowcount-1,5)->text().toFloat());
    if(hBar->value() == hBar->maximum()) //当前hbar在最右侧，即拨到最大值，不断更新
    {

        QDateTime begintime = QDateTime::fromString(ui->tableWidget->item(0,2)->text(),"yyyy-MM-dd HH:mm:ss").addDays((hBar->value()-1)*interval);
        QDateTime newesttime = QDateTime::fromString(ui->tableWidget->item(rowcount-1,2)->text(),"yyyy-MM-dd HH:mm:ss");
        qDebug() << begintime << newesttime << begintime.secsTo(newesttime);
        if(begintime.secsTo(newesttime) >= interval*24*3600)
        {
            hBar->setVisible(true);
            hBar->setMaximum(hBar->maximum()+1);
            hBar->setValue(hBar->maximum());
        }
    }
    //当前hbar不再在最右侧，而在任意其他位置，只更新hbar的最大值
    else
    {
        QDateTime begintime = QDateTime::fromString(ui->tableWidget->item(0,2)->text(),"yyyy-MM-dd HH:mm:ss");
        QDateTime newesttime = QDateTime::fromString(ui->tableWidget->item(ui->tableWidget->rowCount()-1,2)->text(),"yyyy-MM-dd HH:mm:ss");
        qDebug() << begintime.secsTo(newesttime) / (interval*24*3600);
        hBar->setMaximum((begintime.secsTo(newesttime) / (interval*24*3600)) + 1);
    }

}
void StationAcq::updateWaveformWhenScrollBarMove(int value)
{
    int interval;
    switch (ui->comboBox->currentIndex()) {
        case 0:
            interval = 1;  //每日
            break;
        case 1:
            interval = 7;  //每周
            break;
        case 2:
            interval = 30;  //每月
            break;
        default:
            break;
    }
    QDateTime begintime = QDateTime::fromString(ui->tableWidget->item(0,2)->text(),"yyyy-MM-dd HH:mm:ss").addDays(value - 1);
    axisX->setMin(begintime);
    axisX->setMax(begintime.addDays(interval));
}
void StationAcq::updateWaveformDate(int value)
{
    int interval;
    switch (value) {
        case 0:
            interval = 1;  //每日
            break;
        case 1:
            interval = 7;  //每周
            break;
        case 2:
            interval = 30;  //每月
            break;
        default:
            break;
    }
    if(ui->tableWidget->rowCount() >0)
    {
        QDateTime begintime = QDateTime::fromString(ui->tableWidget->item(0,2)->text(),"yyyy-MM-dd HH:mm:ss");
        QDateTime newesttime = QDateTime::fromString(ui->tableWidget->item(ui->tableWidget->rowCount()-1,2)->text(),"yyyy-MM-dd HH:mm:ss");
        if(begintime.secsTo(newesttime) >= interval*24*3600)
        {
            hBar->setVisible(true);
            QDateTime moveingTime = begintime;
            int i=1;
            while(moveingTime.addDays(i).secsTo(newesttime) >= interval*24*3600)
            {
               i++;
            }
            axisX->setMin(moveingTime.addDays(i));
            axisX->setMax(moveingTime.addDays(i+interval));
            hBar->setMaximum(i+1);
            hBar->setValue(i+1);
/*
            for(int i=0; i<ui->tableWidget->rowCount(); i++)
            {
                if(moveingTime.addDays(i).secsTo(QDateTime::fromString(ui->tableWidget->item(i,2)->text(),"yyyy-MM-dd HH:mm:ss")) >= 0)
                {
                   seriesTest->append(QDateTime::fromString(ui->tableWidget->item(i,2)->text(),"yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch(),ui->tableWidget->item(i,5)->text().toFloat());
                }
            }*/
        }
        else
        {
             axisX->setMin(begintime);
             axisX->setMax(begintime.addDays(interval));
             hBar->setVisible(false);
        }
    }
    else
    {
        QDateTime begintime = QDateTime::currentDateTime();
        axisX->setMin(begintime);
        axisX->setMax(begintime.addDays(interval));
        hBar->setVisible(false);
    }
}
void StationAcq::showPointValue(const QPointF &point)
{
    textitem->show();
}
void StationAcq::hidePointValue(const QPointF &point)
{
    textitem->hide();
}
void StationAcq::showHidePoint(const QPointF &point, bool state)
{
    if(state)
    {
        textitem->show();
    }
    else
    {
        textitem->hide();
    }
}
void StationAcq::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
    mTestInitTime = new QLabel(this);
    mTestInitTime->setMinimumWidth(150);
    mTestTime = new QLabel(this);
    mTestTime->setText(tr("测试时长：00:00:00"));
    mTestTime->setMinimumWidth(150);
    mTestNum = new QLabel(this);
    mTestNum->setText(tr("测试周期：0个"));
    mTestNum->setMinimumWidth(120);
    QWidget *widget = new QWidget(this);

    QLabel *netLabel = new QLabel(widget);
    netLabel->setText(tr("网络状态:"));
    led_net = new Led(widget);
    led_net->setColor(Qt::darkGray);
    led_net->setFixedSize(20,20);
    led_net->show();
    QHBoxLayout *widgetLayout = new QHBoxLayout(widget);
    widgetLayout->addWidget(netLabel);
    widgetLayout->addWidget(led_net);
    widgetLayout->setContentsMargins(2,0,2,0);
    widget->setFixedWidth(80);

    statusBar()->addPermanentWidget(mTestInitTime);
    statusBar()->addPermanentWidget(mTestTime);
    statusBar()->addPermanentWidget(mTestNum);
    statusBar()->addPermanentWidget(widget);
}

void StationAcq::createToolBar()
{
    QWidget* spacer = new QWidget();
    QLabel *user = new QLabel();
    user->setText(tr("当前用户 : 管理员 "));
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->toolBar->addWidget(spacer);
    ui->toolBar->addWidget(user);
}
void StationAcq::createSocket()
{
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QIODevice::readyRead, this, &StationAcq::readSocketFeedback);
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(updateConnectState()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(updateConnectState()));
    connect(tcpSocket,SIGNAL(bytesWritten(qint64)),this,SLOT(resendData(qint64)));
}
void StationAcq::resendData(qint64 size)
{
    if(mSend.toLocal8Bit().size() >= mSize)
    {
        mSize += size;
        tcpSocket->write(mSend.toLocal8Bit().right(mSend.toLocal8Bit().size() - mSize));
    }
}
void StationAcq::connectToServer()
{
    if(tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
       tcpSocket->abort();
       tcpSocket->connectToHost(mServerIP,
                                mServerPort);
       statusBar()->showMessage(tr("Connecting"));
    }
}
void StationAcq::reConnectToServer()
{
    tcpSocket->abort();
    tcpSocket->connectToHost(mServerIP,
                             mServerPort);
    statusBar()->showMessage(tr("Connecting"));
}
void StationAcq::updateConnectState()
{
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        led_net->setColor(Qt::green);
        statusBar()->showMessage(tr("Connected"));
//        mainTimer->start();
    }
    else
    {
        led_net->setColor(Qt::darkGray);
        statusBar()->showMessage(tr("Disconnected"));
//        mainTimer->stop();
    }
}
void StationAcq::onMainTimer()
{
//    if(mWorkMode == Idel)
    {
        sendStateAndGetCmd();
    }
}
void StationAcq::sendStateAndGetCmd()
{
    if(!bHeartFlag)
    {
        return;
    }
    connectToServer();
    QStringList list;
    list << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << QString::number(mWorkMode);
    if(mWorkMode == Work || mWorkMode == Calib)
    {
        list << mInitTime.toString("yyyy-MM-dd HH:mm:ss") << QString::number(mTestInterval) << QString::number(mStartTime.secsTo(QDateTime::currentDateTime()));
    }
    QString heartdata = buildSendData(list.join(";"),"HEARTDA");
    if(heartdata.size())
    {
        tcpSocket->write(heartdata.toLocal8Bit());
    }
}
void StationAcq::testcase_socket()
{
    mLatestFile = tr("D:\\测试素材\\测试数据\\水的测试条件016(FD3).TXT");
    mWorkMode = Calib;
    sendAcqData();
}
void StationAcq::readSocketFeedback()
{
    QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());

    QByteArray mContent = tcpSocket->read(tcpSocket->bytesAvailable());
    mReceived += QString::fromLocal8Bit(mContent);
    if(mReceived.size() > 7)
    {
        if(mReceived.mid(2,5).toInt() <= mReceived.toLocal8Bit().size() - 7 - 2 && mReceived.left(2) == tr("ST"))
        {
                eventLog(QString("Send & Get Data Finished!"));
                // decode results and insert table
                qDebug() << mReceived;
                decodeFeedback(mReceived);
         }
    }
/*
    QByteArray mHeader = tcpSocket->read(7);
    if(mHeader.size() == 7 & QString::fromLocal8Bit(mHeader).left(2) == tr("ST"))
    {
        bool ok;
        int length = QString::fromLocal8Bit(mHeader).right(5).toInt(&ok);
        if(ok)
        {
            QByteArray mContent;
            int i=0;
            while(mContent.size() < (length + 2) && i < 10)
            {
               mContent += tcpSocket->read((length + 2) - mContent.size());
               i++;
               SleeperThread::msleep(100);
            }
            if(i >= 10)
            {
                ui->lineEdit->setText("content incomplete error");
            }
            else
            {
                //decode mContent, calc, write
                ui->lineEdit->setText(QString::fromLocal8Bit(mHeader + mContent));

            }
        }
    }
    else
    {
        ui->lineEdit->setText("header error : " + mHeader);
    }
    */
}
void StationAcq::sendAcqData()
{
//    QString filePath = tr("D:\\测试素材\\测试数据\\水的测试条件016(FD3).TXT");
    connectToServer();
    mSend.clear();
    qDebug() << mLatestFile;
    QString filePath = mLatestFile;
    QString mContent = readFile(filePath);
    if(mWorkMode == Work)
    {
        mSend = buildSendData(mContent,"CONTENT");
    }
    else if(mWorkMode == Calib)
    {
        mSend = buildSendData(getCalibResults(filePath),"CALIBRA");
    }
    else if(mWorkMode == Upload)
    {
        mSend = buildSendData(mContent,"UPLOADD");
    }
    else
    {

    }
    if(mSend.size())
    {
        mReceived.clear();
        mSize = 0;
        tcpSocket->write(mSend.toLocal8Bit());
    }
}
QString StationAcq::createBufferFile(QString path)
{

}

QString StationAcq::getCalibResults(QString path)
{
    float result = getPeak(path,mEX,mEM);
    QStringList list;
    list << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")
         << QString::number(mEX)
         << QString::number(mEM)
         << QString::number(mDensity,'f',2)
         << QString::number(mPeak,'f',2)
         << QString::number(result,'f',2);

    qDebug() << list;
    return list.join(';');
}
float StationAcq::getPeak(QString path, int ex, int em)
{
    QTextCodec *code = QTextCodec::codecForName("GBK");
    std::string name = code->fromUnicode(path).data();
    QFile file(QString::fromLocal8Bit(name.c_str()));
//    QFile file(QString::fromLocal8Bit(path.toUtf8()));
    qDebug() << path << QString::fromLocal8Bit(name.c_str());
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "open error";
        return 0;
    }
    QString content = QString::fromLocal8Bit(file.readAll());
    int pos =  content.indexOf("Data Points");
    QString str = content.mid(pos);
    QStringList list = str.split('\n');
    QString xRow = list.at(1);
    xRow.remove(0,1);xRow.remove(xRow.size()-1,1);
    QStringList xList = xRow.split('\t');
    QStringList yList;
    QList<QStringList> valueList;
    for(int i=2; i<list.size();i++)
    {
        QString tmpStr = list.at(i);
        tmpStr.remove(tmpStr.size()-1,1);
        QStringList tmpList = tmpStr.split('\t');
        yList.append(tmpList.at(0));
        tmpList.removeAt(0);
        valueList.append(tmpList);
    }
    qDebug() << xList.size() << yList.size() << valueList.size();

    QString tEX = QString::number(ex) + ".0";
    QString tEM = QString::number(em) + ".0";
    QString tPeak = valueList.at(yList.indexOf(tEM)).at(xList.indexOf(tEX));
    tPeak.remove('\s');
    return tPeak.toFloat();
}
QString StationAcq::calcLength(QString content)
{
    int length = content.toLocal8Bit().size();
    QString sLength = QString::number(length);
    while(sLength.size() < 5)
    {
        sLength.insert(0,'0');
    }
    return sLength;
}
void StationAcq::configDlg()
{
    ConfigDialog dlg;
    dlg.exec();
    readConfig();
}
void StationAcq::uploadDlg()
{
    mWorkMode = Upload;
    UploadDialog *dlg = new UploadDialog;
    connect(dlg,SIGNAL(uploadFile(QString)),this,SLOT(uploadDataFile(QString)));
    connect(this,SIGNAL(uploadFinish(QString)),dlg,SLOT(updateTable(QString)));
    dlg->exec();
    mWorkMode = Idel;
}
void StationAcq::uploadDataFile(QString str)
{
    bHeartFlag = false;
    QFileInfo fileinfo(mConfigFile.readStringContent("DataPath",true),str);
    mLatestFile = fileinfo.absoluteFilePath();
    sendAcqData();
}
void StationAcq::logDlg()
{
    LogDialog dlg;
    dlg.setContent(mContentList);
    dlg.exec();
}
QString StationAcq::readFile(QString filePath)
{
    QString mFileContent;

    QTextCodec *code = QTextCodec::codecForName("GBK");

    QFile file(QString::fromLocal8Bit(code->fromUnicode(filePath)));

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        mFileContent = in.readAll();
    }
    return mFileContent;
}
QString StationAcq::buildSendData(QString content,QString CMD,bool cmdflag)
{
    //tcpSocket->write(QString("ST" + calcLength(send) + send + "ED").toLatin1());
    if(cmdflag)
    {
            content = CMD +":" + content;
    }
    else
    {
       content = "NAME:" + mStationName + CMD +":" + content;
    }


    QString mSendData;
    QString checksum  = QString::number(qChecksum(content.toLocal8Bit().constData(),content.size()),16).toUpper();
    while(checksum.size() < 4)
    {
        checksum.insert(0,'0');
    }
    mSendData = "ST" + calcLength(content + checksum) + content + checksum + "ED";
    return mSendData;
}
void StationAcq::initParams()
{
    readConfig();
    mReceived = "";
    mReadCmdContent = "";
    mTimes = 0;
    mWorkMode = Idel;
    mFileMap.clear();
    bHeartFlag = true;
    bScrollToBottom = true;
}
void StationAcq::readConfig()
{
    mServerIP = mConfigFile.readStringContent("ServerIP",false);
    mServerPort = mConfigFile.readContent("ServerPort").toInt();
    mStationName = mConfigFile.readStringContent("StationName",true);
    mEX = mConfigFile.readContent("EX").toInt();
    mEM = mConfigFile.readContent("EM").toInt();
    mDensity = mConfigFile.readContent("Density").toFloat();
    mPeak = mConfigFile.readContent("Peak").toFloat();
}
void StationAcq::errorLog(QString string)
{
    QFile file("errorlog.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\t" << string << "\n";
}
void StationAcq::eventLog(QString string)
{
    QFile file("eventlog.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream out(&file);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\t" << string << "\n";
}
void StationAcq::testLog(QString string)
{
    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    statusBar()->showMessage(string);
    QFile file("testlog.log");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;

    QTextStream out(&file);
    out << time << "\t" << string << "\n";
    mContentList<< time + "\t" + string;
}

QString StationAcq::getFileRealName(QString name)
{
    //"水的测试条件017.FD3"
    name.replace('.','(');
    name.append(").TXT");
    return name;
}
QString StationAcq::decodeCmd(QString cmd)
{
    QString checksumContent = cmd.mid(7,cmd.size() - 7 - 4 - 2);
    QString checksum = QString::number(qChecksum(checksumContent.toLocal8Bit().constData(),checksumContent.size()),16).toUpper();
    if(cmd.right(2) == "ED")
    {
        if(checksum == cmd.right(6).left(4).right(checksum.size()))
        {
            if(cmd.contains("SENDCMD"))
            {
                QStringList list = cmd.split("SENDCMD:");
                if(list.size() >=2)
                {
                    QString result = list.at(1).mid(0,list.at(1).size() - 6);
                    {
                        if(result == "STATE")
                        {
                            QStringList clist;
                            clist << "STATE" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << QString::number(mWorkMode);
                            if(mWorkMode == Work || mWorkMode == Calib)
                            {
                                clist << mInitTime.toString("yyyy-MM-dd HH:mm:ss") << QString::number(mTestInterval) << QString::number(mStartTime.secsTo(QDateTime::currentDateTime()));
                            }
                            return buildSendData(clist.join(";"),"SENDCMD",true);
                        }
                        else if(result == "START")
                        {
                            bool ret = false;
                            if(mWorkMode == Idel)
                            {
                                setStartMarks();
                                ret = true;
                            }
                            return buildSendData(ret?"START;OK":"START;ER","SENDCMD",true);
                        }
                        else if(result == "STOPP")
                        {
                            bool ret = false;
                            if(mWorkMode == Work)
                            {
                                stop();
                                ret = true;
                            }
                            return buildSendData(ret?"STOPP;OK":"STOPP;ER","SENDCMD",true);
                        }
                        else
                        {

                        }
                    }
                }
            }
        }
    }
    return "";
}
void StationAcq::decodeFeedback(QString content)
{
    QString checksumContent = content.mid(7,content.size() - 7 - 4 - 2);
    QString checksum = QString::number(qChecksum(checksumContent.toLocal8Bit().constData(),checksumContent.size()),16).toUpper();
    if(content.right(2) == "ED")
    {
        if(checksum == content.right(6).left(4).right(checksum.size()))
        {
            if(content.contains("CONTENT"))
            {
                QStringList list = content.split("CONTENT:");
                if(list.size() >=2)
                {
                    QString result = list.at(1).mid(0,list.at(1).size() - 6);
                    QStringList lResult = result.split(';');
                    qDebug() << lResult; //("20", "水的测试条件017.FD3", "2013-04-22 13:37:28", "(230,340)", "2365.0", "(280,320)", "2555.0", "(250,450)", "186.9", "印染废水", "0.97", "0,0,0")
                    if(lResult.size() > 1)
                    {
                       addDataToTable(lResult);
                       updateWaveform();
                       testLog(tr("Test Data Upload Finished!"));

                        QString filename = lResult.at(1) + "&" + lResult.at(2); //getFileRealName(lResult.at(1))
                        if(mFileMap.values().contains(filename))
                        {
                            mFileMap.remove(mFileMap.key(filename));
                        }
                    }
                    else
                    {
                        if(lResult.value(0) == "NoDBName")
                        {
                            QMessageBox::warning(this,tr("提示"),tr("该站点在数据中没有相关信息，请确认再测试。"));

                        }
                    }
                    mReceived = "";
                }
                bHeartFlag = true;
            }
            else if(content.contains("CALIBRA"))
            {
                QStringList list = content.split("CALIBRA:");
                if(list.size() >=2)
                {
                    QString result = list.at(1).mid(0,list.at(1).size() - 6);
                    QStringList lResult = result.split(';');
                    qDebug() << lResult; //("20", "水的测试条件017.FD3", "2013-04-22 13:37:28", "(230,340)", "2365.0", "(280,320)", "2555.0", "(250,450)", "186.9", "印染废水", "0.97", "0,0,0")
                    if(lResult.size() > 0)
                    {

                        if(lResult.value(0) == "NoDBName")
                        {
                            QMessageBox::warning(this,tr("提示"),tr("该站点在数据中没有相关信息，请确认再测试"));

                        }
                        else if(lResult.value(0) == "CalibOK")
                        {
                            testLog(tr("Calibration Data Upload Finished!"));
                        }
                        else if(lResult.value(0) == "CalibER")
                        {
                            testLog(tr("Calibration Data Insert DB Error!"));
                        }
                        else
                        {

                        }
                    }
                    mReceived = "";
                }
                bHeartFlag = true;
            }
            else if(content.contains("UPLOADD"))
            {
                QStringList list = content.split("UPLOADD:");
                if(list.size() >=2)
                {
                    QString result = list.at(1).mid(0,list.at(1).size() - 6);
                    QStringList lResult = result.split(';');
                    qDebug() << lResult; //("20", "水的测试条件017.FD3", "2013-04-22 13:37:28", "(230,340)", "2365.0", "(280,320)", "2555.0", "(250,450)", "186.9", "印染废水", "0.97", "0,0,0")
                    if(lResult.size() > 1)
                    {
                       testLog(tr("Unsend Data Upload Finished!"));
//                       emit uploadFinish(getFileRealName(lResult.at(1)));
                       emit uploadFinish(lResult.at(1) + "&" + lResult.at(2));

                    }
                    else
                    {
                        if(lResult.value(0) == "NoDBName")
                        {
                            QMessageBox::warning(this,tr("提示"),tr("该站点在数据中没有相关信息，请确认再测试"));
                        }
                    }
                    mReceived = "";
                }
                bHeartFlag = true;
            }
            else if(content.contains("HEARTDA"))
            {
                QStringList list = content.split("HEARTDA:");
                if(list.size() >=2)
                {
                    QString result = list.at(1).mid(0,list.at(1).size() - 6);
                    QStringList lResult = result.split(';');
                    qDebug() << lResult; //("SENDCMD", "0/1", "2013-04-22 13:37:28")
                    if(lResult.size() > 1)
                    {
                       qDebug() << (tr("Heart beat bag returned!"));
                       //decode bag content
                    }
                    else
                    {
                        if(lResult.value(0) == "NoDBName")
                        {
                            QMessageBox::warning(this,tr("提示"),tr("该站点在数据中没有相关信息，请确认再测试"));

                        }
                    }
                    mReceived = "";
                }
            }
            else
            {

            }

        }
        else
        {
                    errorLog(QString("Checksum Error : %1").arg(checksum));
        }
    }
    else
    {
        errorLog(QString("End Error"));
    }
}
void StationAcq::addDataToTable(QStringList list)
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
    list.insert(0,QString::number(ui->tableWidget->rowCount()));
    QStringList newList = list.mid(0,4) + list.mid(10,3) + list.mid(4,6);
    newList.swap(2,3);
    for(int i=0; i<ui->tableWidget->columnCount(); i++)
    {
        QTableWidgetItem *item = new QTableWidgetItem(newList.at(i));
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,i,item);
        item->setTextAlignment(Qt::AlignCenter);
    }
    if(bScrollToBottom)
    {
        ui->tableWidget->scrollToBottom();
    }
}
void StationAcq::createConfigFolder()
{
    QDir dir;
    if(!dir.exists("ConfigFiles"))
    {
        dir.mkdir("ConfigFiles");
    }
}
void StationAcq::openServer()
{
    tcpServer = new QTcpServer(this);
    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(getConnection()));
    if (!tcpServer->listen(QHostAddress::Any,mConfigFile.readContent("LocalPort").toInt())) {
        QMessageBox::critical(this, tr("Service Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        return;
    }
}
void StationAcq::getConnection()
{
    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, &QIODevice::readyRead, this, &StationAcq::readCmdContent);
    connect(clientConnection, &QAbstractSocket::disconnected,
            clientConnection, &QObject::deleteLater);
    qDebug() << clientConnection->peerAddress().toString();
}
void StationAcq::readCmdContent()
{
    QTcpSocket* tcpSocket = qobject_cast<QTcpSocket*>(sender());

    QByteArray mContent = tcpSocket->read(tcpSocket->bytesAvailable());
    mReadCmdContent += QString::fromLocal8Bit(mContent);
    if(mReadCmdContent.size() > 7)
    {
        if(mReadCmdContent.mid(2,5).toInt() <= mReadCmdContent.toLocal8Bit().size() - 7 - 2 && mReadCmdContent.left(2) == tr("ST"))
        {
                eventLog(QString("Get Server CMD"));
                qDebug() << "readCmdContent:" << mReadCmdContent;
                //decode CMD and feedback
                QString cmdfeedback = decodeCmd(mReadCmdContent);
                tcpSocket->write(cmdfeedback.toLocal8Bit());
                mReadCmdContent.clear();
         }
    }

}
void StationAcq::openPumpControl()
{
    pumpcontrol->show();
}
QString StationAcq::getProcessNames()
{
    QProcess process;
    process.setReadChannel(QProcess::StandardOutput);
    process.setReadChannelMode(QProcess::MergedChannels);
//    process.start("cmd.exe /C echo test");
    process.start("wmic.exe /OUTPUT:STDOUT PROCESS get Caption");

    process.waitForStarted(1000);
    process.waitForFinished(1000);

    QByteArray list = process.readAll();
    qDebug() << "Read" << list.length() << "bytes";
    QString pstr = QString::fromLatin1(list);
    pstr.remove('\r');
    pstr.remove('\n');
    return pstr;
}
void StationAcq::start()
{
    led_analyze->setColor(Qt::gray);
    led_wait->setColor(Qt::gray);
    led_wash->setColor(Qt::gray);
    led_inputsample->setColor(Qt::gray);
    testLog(tr("-----------------------------------"));
    QString pstr = getProcessNames();
    if(pstr.contains("flsol.exe",Qt::CaseInsensitive))
    {
        mStartTime = QDateTime::currentDateTime();
        mSearchLimit = mConfigFile.readContent("SearchTimeLimit").toInt() * 60;
        updateTestCycle();
        mFilePath = mConfigFile.readStringContent("DataPath",true);
        bAnalyze = false;
        if(!loopTimer->isActive())
        {
            loopTimer->start();
        }
        ui->action_start->setEnabled(false);
        ui->action_upload->setEnabled(false);
        ui->action_calib->setEnabled(false);
        pumpcontrol->start();

        led_inputsample->setColor(Qt::yellow);
        statusBar()->showMessage(mWorkMode == Work ? tr("Testing") : tr("Calibrating"));
        testLog(mWorkMode == Work ? tr("Start Loop Test!") : tr("Start Calibrating!"));
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请打开FL_Solutions程序!"));
        mWorkMode = Idel;
    }
}
void StationAcq::stop()
{
    loopTimer->stop();
    pumpcontrol->haltTest();
    if(bAnalyze)
    {
       stopAnalyze();
    }
    testLog(tr("Stop Loop Test!"));
    ui->action_start->setEnabled(true);
    ui->action_upload->setEnabled(true);
    ui->action_calib->setEnabled(true);
    statusBar()->showMessage(tr("Stop"));
    mWorkMode = Idel;
    saveUnsendFile();
}
//定时启动测试
void StationAcq::onTestLoopTimer()
{
    if(bAnalyze)  //查询文件是否生成
    {
        QString file;
        if(findDataFile(mStartTime,file)) //文件已生成
        {
            emit startwash();
            bHeartFlag = false;
            led_wash->setColor(Qt::yellow);
            stopAnalyze();
            //解析文件            
            testLog(tr("Data File Generated : %1!").arg(file));
            mLatestFile = file;
            if(mWorkMode == Work)
            {
                QFileInfo fileinfo(file) ;
                QStringList list = getFileProperty(fileinfo.absoluteFilePath());
                mFileMap[fileinfo.fileName()] = list.at(0) + "&" + list.at(1);
                //upload and get results;
                sendAcqData();
            }
            else
            {
                //Upload CalibData
                sendAcqData();
            }
        }
        else
        {
            if(mStartTime.secsTo(QDateTime::currentDateTime()) >= mSearchLimit) //查询文件超时
            {
                emit startwash();
                led_wash->setColor(Qt::yellow);
                stopAnalyze();
                //记录错误
                testLog(tr("Data File Generate Error!"));

            }
        }
    }
    int minite = mStartTime.secsTo(QDateTime::currentDateTime())/60;
    int second = mStartTime.secsTo(QDateTime::currentDateTime())%60;

    label_worktime->setText(tr("当前周期运行 %1:%2").arg(minite).arg(buildString(QString::number(second),2)));
    if(mStartTime.secsTo(QDateTime::currentDateTime()) >= mTestInterval)
    {
        led_wait->setColor(Qt::yellow);
        start();
    }
    int hour = mInitTime.secsTo(QDateTime::currentDateTime())/3600;
    minite = (mInitTime.secsTo(QDateTime::currentDateTime()) - hour*60)/60;
    second = mInitTime.secsTo(QDateTime::currentDateTime())%60;  //

    //测试时长 : 00:00:00
    mTestTime->setText(tr("测试时长 : %1:%2:%3").arg(buildString(QString::number(hour),2)).arg(buildString(QString::number(minite),2)).arg(buildString(QString::number(second),2)));
}
void StationAcq::startAnalyze()
{
    SimulateKeyPress(VK_F4);
    bAnalyze = true;
    testLog(tr("Start Analyze!"));
    led_analyze->setColor(Qt::yellow);
    led_inputsample->setColor(Qt::green);
}
void StationAcq::stopAnalyze()
{
    SimulateKeyPress(VK_F6);
    bAnalyze = false;
    testLog(tr("Stop Analyze!"));
    led_analyze->setColor(Qt::green);
}
void StationAcq::finish()
{
    if(mConfigFile.readContent("Trigger").toString() == "Single" || mWorkMode == Calib)
    {
        ui->action_start->setEnabled(true);
        ui->action_upload->setEnabled(true);
        ui->action_calib->setEnabled(true);
        loopTimer->stop();
        led_wait->setColor(Qt::green);
        statusBar()->showMessage(mWorkMode == Calib ? tr("Calibration Finished!") : tr("Test Finished!"));
        mWorkMode = Idel;
        saveUnsendFile();
    }
    else
    {
        led_wait->setColor(Qt::yellow);
    }
    testLog(mWorkMode == Calib ? tr("Calibration Finished!") : tr("Test Finished!"));
    led_wash->setColor(Qt::green);
    mTimes++ ;

    mTestNum->setText(tr("测试周期 : %1个").arg(mTimes));
}
bool StationAcq::findDataFile(QDateTime starttime, QString &file)
{
    bool ret = false;
    QDir dir(mFilePath);
    QStringList filters;
    filters.append("*.txt");
    QFileInfoList list = dir.entryInfoList(filters,QDir::Files,QDir::Time);
    if(list.size())
    {
        if(list.at(0).created() > starttime)
        {
            file = list.at(0).filePath();
            QFile fileinfo(file);
            ret = fileinfo.size() > 0;
        }
    }
    return ret;
}
void StationAcq::testcase()
{

}
void StationAcq::updateTestCycle()
{
    mTestInterval = mConfigFile.readContent("TriggerInterval").toInt() * 60;
    label_cycle->setText(tr("工作周期 %1:00").arg(mConfigFile.readContent("TriggerInterval").toInt()));
}
QString StationAcq::buildString(QString input, int length)
{
    while(input.size() < length)
    {
        input.insert(0,'0');
    }
    return input;
}
void StationAcq::createIndicator()
{
    ui->frame_4->setFixedHeight(30);
    led_wash  = new Led(ui->frame_4);
    led_wash->setColor(Qt::gray);
    led_wash->show();
    led_wash->setMinimumSize(20,20);
    led_analyze = new Led(ui->frame_4);
    led_analyze->setColor(Qt::gray);
    led_analyze->show();
    led_analyze->setMinimumSize(20,20);
//    led_analyze->setFixedHeight(31);
    led_inputsample = new Led(ui->frame_4);
    led_inputsample->setColor(Qt::gray);
    led_inputsample->show();
    led_inputsample->setMinimumSize(20,20);
//    led_inputsample->setFixedHeight(31);
    led_wait = new Led(ui->frame_4);
    led_wait->setColor(Qt::gray);
    led_wait->show();
    led_wait->setMinimumSize(20,20);
//    led_wait->setFixedHeight(31);

    label_inputsample = new QLabel(ui->frame_4);
    label_inputsample->setText(tr("进样指示灯:"));
    label_analyze = new QLabel(ui->frame_4);
    label_analyze->setText(tr("分析指示灯:"));
    label_wash = new QLabel(ui->frame_4);
    label_wash->setText(tr("清洗指示灯:"));
    label_wait = new QLabel(ui->frame_4);
    label_wait->setText(tr("等待指示灯:"));

    label_cycle = new QLabel(ui->frame_4);
    label_worktime = new QLabel(ui->frame_4);
    label_worktime->setText(tr("当前周期运行 0:00"));

    QHBoxLayout *ledlayout = new QHBoxLayout(ui->frame_4);
    ledlayout->addWidget(label_cycle);
    ledlayout->addWidget(label_worktime);
    ledlayout->addStretch();
    ledlayout->addWidget(label_inputsample);
    ledlayout->addWidget(led_inputsample);
    ledlayout->addWidget(label_analyze);
    ledlayout->addWidget(led_analyze);
    ledlayout->addWidget(label_wash);
    ledlayout->addWidget(led_wash);
    ledlayout->addWidget(label_wait);
    ledlayout->addWidget(led_wait);
    ledlayout->setContentsMargins(6,1,6,1);
//    ledlayout->setMargin(6);
}
void StationAcq::setStartMarks()
{
    mTimes = 0;
    mInitTime = QDateTime::currentDateTime();
    mTestTime->setText(tr("测试时长: 00:00:00"));
    mTestNum->setText(tr("测试周期: 0个"));
    mWorkMode = Work;
    mFileMap.clear();
    start();
    mTestInitTime->setText(tr("测试起始时间: %1").arg(mInitTime.toString("yyyy-MM-dd HH:mm:ss")));
    hBar->setMaximum(1);
    hBar->setValue(1);
    hBar->setVisible(false);
    seriesTest->clear();
}
void StationAcq::setCalibMarks()
{
    mTimes = 0;
    mInitTime = QDateTime::currentDateTime();
    mTestTime->setText(tr("测试时长：00:00:00"));
    mTestNum->setText(tr("测试周期：0个"));
    mWorkMode = Calib;
    start();
}
void StationAcq::saveUnsendFile()
{
    if(mFileMap.size())
    {
        QFile file("unsendfiles.dat");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
            return;

        QTextStream out(&file);
        for(int i=0;i<mFileMap.size();i++)
        {
           out << mFileMap.values().at(i) << "\n";
        }
        mFileMap.clear();

    }
}
void StationAcq::testcase_settablevalue()
{
    seriesTest->clear();
    ////("20", "水的测试条件017.FD3", "2013-04-22 13:37:28", "(230,340)", "2365.0", "(280,320)", "2555.0", "(250,450)", "186.9", "印染废水", "0.97", "0,0,0")
    srand((unsigned)time(NULL));
    for(int i=0; i<30; i++)
    {
        QStringList list;
        list << QString::number(i+1) << tr("水的测试条件%1.FD3").arg(i+1) << QDateTime::currentDateTime().addSecs(i*3600).toString("yyyy-MM-dd HH:mm:ss")
             << "(230,340)" <<   "2365.0" << "(280,320)" << "2555.0" << "(250,450)" << "186.9" << "印染废水"
             << QString::number(((double)rand())/RAND_MAX,'f',3) << "0,0,0";

        addDataToTable(list);
        updateWaveform();
//        seriesTest->append(QDateTime::fromString(ui->tableWidget->item(i,2)->text(),"yyyy-MM-dd HH:mm:ss").toMSecsSinceEpoch(),ui->tableWidget->item(i,5)->text().toFloat());
    }
}
void StationAcq::testcase_ontimer()
{

    QStringList list;
    list << QString::number(ui->tableWidget->rowCount()+1) << tr("水的测试条件%1.FD3").arg(ui->tableWidget->rowCount()+1) << QDateTime::currentDateTime().addSecs(ui->tableWidget->rowCount()*3600).toString("yyyy-MM-dd HH:mm:ss")
         << "(230,340)" <<   "2365.0" << "(280,320)" << "2555.0" << "(250,450)" << "186.9" << "印染废水"
         << QString::number(((double)rand())/RAND_MAX,'f',3) << "0,0,0";

    addDataToTable(list);
    updateWaveform();
    qDebug() << ui->tableWidget->rowCount();
}
void StationAcq::showTableValue(int value)
{
    qDebug() << value << ui->tableWidget->verticalScrollBar()->maximum();
    bScrollToBottom = ui->tableWidget->verticalScrollBar()->maximum() == value;
}
QStringList StationAcq::getFileProperty(QString filename)
{
    QStringList lproperty;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return lproperty;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if(line.contains("File name",Qt::CaseInsensitive))
        {
            QStringList list = line.split('\t');

            lproperty.append(list.at(1));
        }
        if(line.contains("Run Date",Qt::CaseInsensitive))
        {
            QStringList list = line.split('\t');
            QDateTime datetime = QDateTime::fromString(list.at(1),"HH:mm:ss, MM/dd/yyyy");
            lproperty.append(datetime.toString("yyyy-MM-dd HH:mm:ss"));
            break;
        }
    }
    return lproperty;
}

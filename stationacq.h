#ifndef STATIONACQ_H
#define STATIONACQ_H

#include <QMainWindow>
#include "chartview.h"
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QGraphicsLineItem>
#include <QDateTimeAxis>
#include <QScatterSeries>
#include <QProgressBar>
#include <QLabel>
#include "led.h"
#include <QScrollBar>
#include <QTcpSocket>
#include <QMap>
#include "windows.h"
#include "configmanagement.h"
#include <QTcpServer>
#include "pumpcontroldlg.h"
#include <QDateTime>
#include "led.h"
#include <QLabel>

enum WorkMode
{
    Idel = 0,
    Work,
    Upload,
    Calib
};

namespace Ui {
class StationAcq;
}

class StationAcq : public QMainWindow
{
    Q_OBJECT

public:
    explicit StationAcq(QWidget *parent = 0);
    ~StationAcq();
private:
    void setTableHeader();
    void setWavefrom();
    void createStatusBar();
    void createToolBar();
    void createSocket();
    void initParams();
    void errorLog(QString string);
    void eventLog(QString string);
    void testLog(QString string);
    QString calcLength(QString);
    QString readFile(QString);
    QString buildSendData(QString content,QString CMD,bool cmdflag = false);
    void decodeFeedback(QString feedback);
    QString decodeCmd(QString cmd);
    void addDataToTable(QStringList);
    void createConfigFolder();
    void openServer();
    QString getProcessNames();
    void updateTestCycle();
    void testcase();
    QString buildString(QString,int);
    void createIndicator();
    void readConfig();
    QString createBufferFile(QString path);
    float getPeak(QString,int,int);
    void saveUnsendFile();
    QString getFileRealName(QString);
    void sendStateAndGetCmd();
    void testcase_waveform();    
    void testcase_settablevalue();
    QStringList getFileProperty(QString filename);
private slots:
    void testcase_socket();
    void readSocketFeedback();
    void sendAcqData();
    void configDlg();
    void logDlg();
    void uploadDlg();
    void resendData(qint64);
    void SimulateKeyPress(uint keyCode);
    void startFlsolProcess();
    void getConnection();
    void readCmdContent();
    void testSimulateKeyPress();
    void openPumpControl();
    void connectToServer();
    void reConnectToServer();
    void updateConnectState();
    void start();
    void stop();

    void onTestLoopTimer();
    void onMainTimer();
    void startAnalyze();
    void stopAnalyze();
    void finish();
    bool findDataFile(QDateTime,QString&);
    void setStartMarks();
    void setCalibMarks();
    QString getCalibResults(QString);
    void showPointValue(const QPointF &point);
    void hidePointValue(const QPointF &point);
    void showHidePoint(const QPointF &point,bool state);
    void updateWaveform();
    void updateWaveformWhenScrollBarMove(int);
    void updateWaveformDate(int value);
    void testcase_ontimer();
    void showTableValue(int);
public slots:
    void uploadDataFile(QString);
signals:
    void startwash();
    void uploadFinish(QString);
private:
    Ui::StationAcq *ui;
    ChartView *chartview;
    QChart *chart;
    QValueAxis *axisY;
    QDateTimeAxis *axisX;
    QGraphicsTextItem *textitem;
    QLineSeries *seriesTest;
    QLabel *mTestNum,*mTestTime,*mTestInitTime;

    QScrollBar *hBar;
    int mTimes;
    QDateTime mInitTime;
    QTcpSocket *tcpSocket;
    QString mStationName;
    QString mSend,mReceived,mReadCmdContent;
    qint64 mSize;
    WorkMode mWorkMode; // Idel,Work,Upload,Calib
    ConfigManagement mConfigFile;
    QString mServerIP;
    int mServerPort;
    QTcpServer *tcpServer;
    PumpControlDlg *pumpcontrol;
    QTimer *loopTimer, *mainTimer;
    int mEX,mEM;
    float mDensity,mPeak;
    bool bHeartFlag;
    /******* Test Loop Params *****/
    QDateTime mStartTime;
    bool bAnalyze,bScrollToBottom;
    int mTestInterval,mSearchLimit;
    QString mLatestFile,mFilePath;
    Led *led_inputsample,*led_wash,*led_analyze,*led_wait,*led_net;
    QLabel *label_cycle,*label_worktime,*label_inputsample,*label_wash,*label_analyze,*label_wait;
    QStringList mContentList;
    QMap<QString,QString> mFileMap;
};

#endif // STATIONACQ_H

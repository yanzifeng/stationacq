#ifndef DEF_H
#define DEF_H
#include <QVector>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QDateTime>
#include <QSemaphore>

//环境参数全局变量结构体
struct sEnv{

    float StationTemp;
    float CarTemp;
    float WindSpeed;
    float WindDir;
    float Lat;
    float Lon;
    QString Lat_Prefix;
    QString Lon_Prefix;
};

struct sAirTest{
    int mNum;
    QMap<QString,QStringList> mContent; // id : type,name,density,value,state(当前是否报警)
    int mSelected;
    QStringList orderedID;
};

extern int eAirState; // 0--采集外部气体；1--选用预设气体
extern int ePreSetAirID;
extern sEnv eEnvValue;
extern sAirTest eAirTestValue;
extern QMap<QString,QStringList> eAirRTValue; // id : type,name,density,value,first_grade,second_grade,bomb_limit,warning1,warning2,advice
extern QDateTime eStartTime;
extern QSemaphore semaphore;
extern bool stop_flag;

#endif // DEF_H

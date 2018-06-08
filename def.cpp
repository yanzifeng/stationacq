#include "def.h"

int eAirState = 0; // 0--采集外部气体；1--选用预设气体
int ePreSetAirID;
sEnv eEnvValue;
sAirTest eAirTestValue;
QMap<QString,QStringList> eAirRTValue;
QDateTime eStartTime;
QSemaphore semaphore;
bool stop_flag;

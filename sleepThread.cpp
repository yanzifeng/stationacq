#include "sleepThread.h"

void SleeperThread::msleep(unsigned long msecs)
{
	QThread::msleep(msecs);
}
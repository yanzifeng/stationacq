#ifndef SLEEPTHREAD_H
#define SLEEPTHREAD_H

#include <QThread>

class SleeperThread : public QThread
{
public:
static void msleep(unsigned long msecs);
};

#endif

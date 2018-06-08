#include "stationacq.h"
#include <QApplication>
#include "configdialog.h"
#include "logdialog.h"
#include "uploaddialog.h"
#include "logindialog.h"
int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);

//    UploadDialog w;
    StationAcq w;
//    LogDialog w;
//    ConfigDialog w;
//    LoginDialog w;
    w.show();

    return a.exec();
}

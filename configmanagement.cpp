#include "configmanagement.h"
#include <QFile>
#include <QTextCodec>

ConfigManagement::ConfigManagement()
{
    setting = new QSettings("ConfigFiles/Config.ini",QSettings::IniFormat);
    setting->beginGroup("Params");
    setting->setIniCodec(QTextCodec::codecForName("GBK"));
}
QVariant ConfigManagement::readContent(QString name)
{
    if(QFile::exists("ConfigFiles/Config.ini"))
    {
        return setting->value(name);
    }
    else
    {
        return QVariant();
    }
}
QString ConfigManagement::readStringContent(QString name, bool encode)
{
    if(QFile::exists("ConfigFiles/Config.ini"))
    {
        if(encode)
        {
            return QString::fromUtf8(setting->value(name).toString().toLocal8Bit());
        }
        else
        {
            return setting->value(name).toString();
        }
    }
    else
    {
        return "";
    }
}

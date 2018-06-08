#ifndef CONFIGMANAGEMENT_H
#define CONFIGMANAGEMENT_H
#include <QSettings>
#include <QVariant>
#include <QString>

class ConfigManagement
{
public:
    ConfigManagement();
    QString readStringContent(QString,bool);  //TRUE : Encode , False : No
    QVariant readContent(QString);
private:
    QSettings *setting;
};

#endif // CONFIGMANAGEMENT_H

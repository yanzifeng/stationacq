#include "logdialog.h"
#include "ui_logdialog.h"

LogDialog::LogDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint);  //Qt::WindowMinimizeButtonHint
}

LogDialog::~LogDialog()
{
    delete ui;
}
void LogDialog::setContent(QStringList list)
{
    ui->textEdit->clear();
    for(int i=0; i<list.size(); i++)
    {
        ui->textEdit->append(list.at(i));
    }
}

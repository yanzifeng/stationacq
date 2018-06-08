#include "uploaddialog.h"
#include "ui_uploaddialog.h"
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>

UploadDialog::UploadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UploadDialog)
{
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
    ui->setupUi(this);
    setWindowModality(Qt::NonModal);
    connect(ui->pushButton_select,SIGNAL(clicked(bool)),this,SLOT(selectAll()));
    connect(ui->pushButton_diselect,SIGNAL(clicked(bool)),this,SLOT(diselectAll()));
    connect(ui->pushButton_upload,SIGNAL(clicked(bool)),this,SLOT(upload()));
    connect(ui->pushButton_stop,SIGNAL(clicked(bool)),this,SLOT(setFlagFalse()));
    ui->pushButton_upload->setEnabled(false);
    ui->pushButton_diselect->setEnabled(false);
    ui->pushButton_select->setEnabled(false);
    ui->pushButton_stop->setEnabled(false);

    initTable();
    loadTableData();
}

UploadDialog::~UploadDialog()
{
    delete ui;
}
void UploadDialog::initTable()
{
    QStringList headers;
    headers << tr("序号") << tr("文件名") << tr("选择") << tr("上传状态");

    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setVisible(true);
    QList<int> colWidth;
    colWidth << 50 << 250 << 50 << 50;
    for(int i=0; i<colWidth.size(); i++)
    {
        ui->tableWidget->setColumnWidth(i,colWidth.at(i));
    }

}
void UploadDialog::updateTable(QString str)
{
    int idx = mFileNameList.indexOf(str);
    qDebug() << str << idx;
    ui->tableWidget->item(idx,3)->setBackgroundColor(Qt::green);
    qDebug() << pSelectIndex << idx;
    pSelectIndex.removeAt(pSelectIndex.indexOf(idx));
    saveUnsendFiles();
    if(pSelectIndex.size())
    {
        if(mFlag)
        {
            qDebug() <<"UploadDLG Upload file:" <<ui->tableWidget->item(pSelectIndex.at(0),1)->text();
            emit uploadFile(ui->tableWidget->item(pSelectIndex.at(0),1)->text());
        }
        else
        {
            ui->pushButton_upload->setEnabled(true);
            ui->pushButton_diselect->setEnabled(true);
            ui->pushButton_select->setEnabled(true);
            ui->pushButton_stop->setEnabled(false);
        }
    }
    else
    {
//        QMessageBox::information(this,tr("提示"),tr("上传完毕!"));
        ui->pushButton_upload->setEnabled(true);
        ui->pushButton_diselect->setEnabled(true);
        ui->pushButton_select->setEnabled(true);
        ui->pushButton_stop->setEnabled(false);
    }
}
void UploadDialog::loadTableData()
{
    QFile file("unsendfiles.dat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QString path = mConfig.readStringContent("DataPath",true);
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QFileInfo fileinfo(QDir(path), line);
        if(QFile::exists(fileinfo.absoluteFilePath()))
        {
            mList.append(line);
            qDebug() << "Valid File : " << fileinfo.absoluteFilePath();
            QStringList list = getFileProperty(fileinfo.absoluteFilePath());
            mFileNameList.append(list.at(0) + "&" + list.at(1));
        }
        else
        {
            qDebug() << "Missing File : " << fileinfo.absoluteFilePath();
        }
    }
    qDebug() << mFileNameList;
    if(!mList.size())
        return;
    ui->tableWidget->setRowCount(mList.size());
    for(int i=0; i< mList.size();i++) // column 1
    {
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(i+1));
        ui->tableWidget->setItem(i,0,item);
        item->setTextAlignment(Qt::AlignCenter);
    }
    for(int i=0; i< mList.size();i++) // column 2
    {
        QTableWidgetItem *item = new QTableWidgetItem(mList.at(i));
        ui->tableWidget->setItem(i,1,item);
        item->setTextAlignment(Qt::AlignCenter);
    }
    for(int i=0; i< mList.size();i++) // column 3
    {
//        QTableWidgetItem *item = new QTableWidgetItem();
//        ui->tableWidget->setItem(i,2,item);
//        item->setTextAlignment(Qt::AlignCenter);
//        item->setCheckState(Qt::Unchecked);
        QWidget *pWidget = new QWidget();
        QCheckBox *pCheckBox = new QCheckBox();
        pCheckBox->setWhatsThis(tr("Row %1").arg(i+1));
        connect(pCheckBox,SIGNAL(stateChanged(int)),this,SLOT(updateSelection(int)));
        connect(this,SIGNAL(doSelect(bool)),pCheckBox,SLOT(setChecked(bool)));
        QHBoxLayout *pLayout = new QHBoxLayout(pWidget);
        pLayout->addWidget(pCheckBox);
        pLayout->setAlignment(Qt::AlignCenter);
        pLayout->setContentsMargins(0,0,0,0);
        pWidget->setLayout(pLayout);
        ui->tableWidget->setCellWidget(i,2,pWidget);
        mSelection << false;
    }
    for(int i=0; i< mList.size();i++) // column 4
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        ui->tableWidget->setItem(i,3,item);
        item->setTextAlignment(Qt::AlignCenter);
        item->setBackgroundColor(Qt::darkGray);
    }
    ui->pushButton_upload->setEnabled(true);
    ui->pushButton_diselect->setEnabled(true);
    ui->pushButton_select->setEnabled(true);
}
void UploadDialog::updateSelection(int state)
{
    QCheckBox *pCheckBox = qobject_cast<QCheckBox*>(sender());
    qDebug() << pCheckBox->whatsThis() <<pCheckBox->whatsThis().mid(4).toInt();
    int pIndex = pCheckBox->whatsThis().mid(4).toInt() - 1;
    if(state == Qt::Unchecked)
    {
        mSelection.replace(pIndex,false);
    }
    else
    {
        mSelection.replace(pIndex,true);
    }
    qDebug() << mSelection;
}
void UploadDialog::diselectAll()
{
    emit doSelect(false);
}
void UploadDialog::selectAll()
{
    emit doSelect(true);
}
void UploadDialog::upload()
{
    pSelectIndex.clear();
    for(int i=0;i<mList.size();i++)
    {
        if(mSelection.at(i) & ui->tableWidget->item(i,3)->backgroundColor() != Qt::green)
        {
            pSelectIndex.append(i);
        }
    }
    if(pSelectIndex.size())
    {
        ui->pushButton_upload->setEnabled(false);
        ui->pushButton_diselect->setEnabled(false);
        ui->pushButton_select->setEnabled(false);
        ui->pushButton_stop->setEnabled(true);
        mFlag = true;
        emit uploadFile(ui->tableWidget->item(pSelectIndex.at(0),1)->text());
    }
    else
    {
        QMessageBox::information(this,tr("提示"),tr("请先选择要上传的文件!"));
    }
}
void UploadDialog::saveUnsendFiles()
{
    qDebug() << "enter save unsend files";
    QStringList newList;
    for(int i=0;i < ui->tableWidget->rowCount();i++)
    {
        if(ui->tableWidget->item(i,3)->backgroundColor() != Qt::green)
        {
            newList.append(ui->tableWidget->item(i,1)->text());
        }
    }
    qDebug() << newList;
    if(newList.size())
    {
        QFile file("unsendfiles.dat");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);
        for(int i=0;i<newList.size();i++)
        {
           out << newList.at(i) << "\n";
        }
    }
    else
    {
        QFile file("unsendfiles.dat");
        file.remove();
    }
}
void UploadDialog::setFlagFalse()
{
    mFlag = false;
}
QStringList UploadDialog::getFileProperty(QString filename)
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

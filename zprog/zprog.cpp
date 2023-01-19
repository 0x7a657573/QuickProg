#include "zprog.h"
#include <QDebug>
#include "mtkprog.h"
#include <QImage>
zprog::zprog(int id,QWidget *parent)
    : QWidget{parent}
{
    Layout = new QVBoxLayout(this);

    HLay = new QHBoxLayout();
    imgStatus = new QLabel("");

    imgStatus->setPixmap(QPixmap::fromImage(QImage(":/Icon/wait")).scaled(16,16,Qt::KeepAspectRatio));
    imgStatus->adjustSize();
    Progress = new QProgressBar(this);

    HLay->addWidget(imgStatus);
    HLay->addWidget(Progress);

    log_view = new QPlainTextEdit(this);
    log_view->setReadOnly(true);

    Layout->addLayout(HLay);
    Layout->addWidget(log_view);
    PowerContorl = false;
    xid = id;
}

void zprog::SetBaud(int baud)
{
    xBaud = baud;
}

void zprog::SetPowerContorl(bool Enable,bool Is_DTR,bool Is_Not)
{
    PowerContorl = Enable;
    IsPcDTR = Is_DTR;
    IsPcNot = Is_Not;
}

void zprog::SetSerialPort(const QString PortName)
{
    xPortName = PortName;
    writelog("Set "+PortName);
}

void zprog::SetfirmwarePath(const QString firmwarePath)
{
    xFirmwarePath = firmwarePath;
}


void zprog::start(void)
{
    QThread *thread;
    mtkprog *mtkworker;

    log_view->clear();
    /*update status*/
    imgStatus->setPixmap(QPixmap::fromImage(QImage(":/Icon/wait")).scaled(16,16,Qt::KeepAspectRatio));

    thread = new QThread();
    mtkworker = new mtkprog(xPortName,xFirmwarePath,PowerContorl,IsPcDTR,IsPcNot);

    switch (xBaud) {
    case 115200:
        mtkworker->setBaud(mtkprog::UART_BAUD_115200);
        break;
    case 230400:
        mtkworker->setBaud(mtkprog::UART_BAUD_230400);
        break;
    case 460800:
        mtkworker->setBaud(mtkprog::UART_BAUD_460800);
        break;
    case 921600:
        mtkworker->setBaud(mtkprog::UART_BAUD_921600);
        break;
    default:
        mtkworker->setBaud(mtkprog::UART_BAUD_460800);
        break;
    }

    mtkworker->moveToThread(thread);

    connect(mtkworker, &mtkprog::progress,this , &zprog::setProgress);
    connect(mtkworker, &mtkprog::wlog,this , &zprog::writelog);
    connect(mtkworker, &mtkprog::finishedWithStatus, this, &zprog::finished);

    //connect( worker, &mtkprog::error, this, &zprog::errorString);
    connect( thread, &QThread::started, mtkworker, &mtkprog::Start);
    connect( mtkworker, &mtkprog::finished, thread, &QThread::quit);
    connect( mtkworker, &mtkprog::finished, mtkworker, &mtkprog::deleteLater);
    connect( thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();




}

void zprog::finished(bool hasError)
{
    /*check status*/
    if(!hasError)
        imgStatus->setPixmap(QPixmap::fromImage(QImage(":/Icon/ok")).scaled(16,16,Qt::KeepAspectRatio));
    else
        imgStatus->setPixmap(QPixmap::fromImage(QImage(":/Icon/error")).scaled(16,16,Qt::KeepAspectRatio));
    emit ended(xid);
}

void zprog::writelog(QString str)
{
    log_view->appendPlainText(str);
}

void zprog::setProgress(int value)
{
   Progress->setValue(value);
}





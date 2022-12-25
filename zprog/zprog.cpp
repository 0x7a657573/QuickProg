#include "zprog.h"
#include <QDebug>

zprog::zprog(QWidget *parent)
    : QWidget{parent}
{
    Layout = new QVBoxLayout(this);

    Progress = new QProgressBar(this);
    log_view = new QPlainTextEdit(this);
    log_view->setReadOnly(true);

    Layout->addWidget(Progress);
    Layout->addWidget(log_view);


}

void zprog::SetSerialPort(const QString PortName)
{
    SerialPort.setPortName(PortName);
}

void zprog::start(void)
{
    QThread *thread1 = QThread::create(std::bind(&zprog::DoPrograming,this));
    thread1->start();
}

void zprog::setProgress(int value)
{
   Progress->setValue(value);
}

void zprog::DoPrograming(void)
{
    SerialPort.open(QIODeviceBase::ReadWrite);
    for(int i=0;i<=100;i++)
    {
        emit setProgress(i);
        SerialPort.write("Salam\r\n");
        //qDebug() << "Start This Thred" << ;
        QThread::msleep(50);
    }
    SerialPort.close();
}





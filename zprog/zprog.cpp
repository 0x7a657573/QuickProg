#include "zprog.h"
#include <QDebug>
#include "mtkprog.h"

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
    xPortName = PortName;

}

void zprog::start(void)
{
    QThread *thread;
    mtkprog *mtkworker;

    thread = new QThread();
    mtkworker = new mtkprog(xPortName);
    mtkworker->moveToThread(thread);

    connect(mtkworker, &mtkprog::progress,this , &zprog::setProgress);
    connect(mtkworker, &mtkprog::wlog,this , &zprog::writelog);

    //connect( worker, &mtkprog::error, this, &zprog::errorString);
    connect( thread, &QThread::started, mtkworker, &mtkprog::Start);
    connect( mtkworker, &mtkprog::finished, thread, &QThread::quit);
    connect( mtkworker, &mtkprog::finished, mtkworker, &mtkprog::deleteLater);
    connect( thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void zprog::writelog(QString str)
{
    log_view->appendPlainText(str);
}

void zprog::setProgress(int value)
{
   Progress->setValue(value);
}





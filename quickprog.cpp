#include "quickprog.h"
#include "ui_quickprog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>

QuickProg::QuickProg(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QuickProg)
{
    ui->setupUi(this);
    QVBoxLayout *main_lay = new QVBoxLayout(this);
    QHBoxLayout *Toolbarlay = new QHBoxLayout(this);
    LoadToolBar(Toolbarlay);
    Toolbarlay->setAlignment(Qt::AlignLeft);


    programmer = new zprog(this);



    main_lay->addLayout(Toolbarlay);
    main_lay->addWidget(programmer);


    QWidget *widget = new QWidget();
    widget->setLayout(main_lay);
    setCentralWidget(widget);

    //setCentralWidget(programmer);
}



QuickProg::~QuickProg()
{
    delete ui;
}



void QuickProg::LoadToolBar(QHBoxLayout *lay)
{
    /*Serial Port List*/
    xPort = new QComboBox(this);
    xPort->setFixedHeight(30);
    xPort->setToolTip(tr("Select Serial Port"));

    /*Baud Rate*/
    int Bauds[] = {2400,4800,9600,19200,38400,57600,115200,230400,460800,921600};
    xBaud = new QComboBox(this);
    xBaud->setFixedHeight(30);
    xBaud->setToolTip(tr("set BaudRate"));
    for(long unsigned int i=0;i<(sizeof(Bauds)/sizeof(int));i++)
    {
       xBaud->addItem( QString::number(Bauds[i]) );
    }
    xBaud->setCurrentIndex(6); /*Set in 115200*/

    /*start btn*/
    QPushButton *btnStart = new QPushButton(this);
    btnStart->setAutoFillBackground(true);
    //btnStart->setFixedSize(30,30);
    btnStart->setToolTip(tr("Start Programming"));
    btnStart->setText(tr("Start"));
    connect(btnStart,SIGNAL(clicked()),this,SLOT(handel_StartAction()));

    QSpacerItem *SendSpitem = new QSpacerItem(10,0, QSizePolicy::Fixed, QSizePolicy::Minimum);

    lay->addWidget(xPort);
    lay->addWidget(xBaud);
    lay->addSpacerItem(SendSpitem);
    lay->addWidget(btnStart);

    /*load serial port*/
    handel_LoadSerialPort();
}

void QuickProg::handel_StartAction()
{
    programmer->SetSerialPort(xPort->currentText());
    programmer->start();
}


void QuickProg::handel_LoadSerialPort()
{
    xPort->clear();
    int i=0;
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts())
    {
        xPort->addItem(port.portName());
        if(i==0) xPort->setToolTip((port.description()=="") ? "None":port.description());
        xPort->setItemData(i++,(port.description()=="") ? "None":port.description(),Qt::ToolTipRole);
    }
}

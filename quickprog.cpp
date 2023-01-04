#include "quickprog.h"
#include "ui_quickprog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QFileDialog>

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
    btnStart->setFixedSize(30,30);
    btnStart->setToolTip(tr("Start Programming"));
    btnStart->setIcon(QIcon(":/Icon/runing"));
    connect(btnStart,SIGNAL(clicked()),this,SLOT(handel_StartAction()));

    QPushButton *btnConfig = new QPushButton(this);
    btnConfig->setAutoFillBackground(true);
    btnConfig->setFixedSize(30,30);
    btnConfig->setToolTip(tr("Software Setting"));
    btnConfig->setIcon(QIcon(":/Icon/ico_setting"));
    connect(btnConfig,SIGNAL(clicked()),this,SLOT(handel_SettingAction()));

    /*browse btn*/
    QPushButton *btnBrowse = new QPushButton(this);
    btnBrowse->setAutoFillBackground(true);
    btnBrowse->setFixedSize(30,30);
    btnBrowse->setToolTip(tr("Select firmware"));
    btnBrowse->setIcon(QIcon(":/Icon/browse_file"));
    connect(btnBrowse,SIGNAL(clicked()),this,SLOT(handel_BrowseFile()));

    LPath = new QLineEdit(this);
    LPath->setReadOnly(true);
    LPath->setMinimumSize(150,30);


    QSpacerItem *SendSpitem = new QSpacerItem(10,0, QSizePolicy::Fixed, QSizePolicy::Minimum);

    lay->addWidget(xPort);
    lay->addWidget(xBaud);
    lay->addWidget(LPath);
    lay->addWidget(btnBrowse);
    lay->addWidget(btnStart);
    lay->addSpacerItem(SendSpitem);

    lay->addWidget(btnConfig);

    /*load serial port*/
    handel_LoadSerialPort();
}

void QuickProg::handel_SettingAction()
{
    qDebug() << "hoo";
}

void QuickProg::handel_BrowseFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    if(fileName != "")
    {
        LPath->setText(fileName);
        programmer->SetfirmwarePath(fileName);
    }
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
        //qDebug() << port.productIdentifier() << port.vendorIdentifier();
        xPort->addItem(port.portName());
        if(i==0) xPort->setToolTip((port.description()=="") ? "None":port.description());
        xPort->setItemData(i++,(port.description()=="") ? "None":port.description(),Qt::ToolTipRole);
    }
}

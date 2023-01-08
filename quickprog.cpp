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

    /*load setting*/
    LoadSetting();

    QVBoxLayout *main_lay = new QVBoxLayout();
    QHBoxLayout *Toolbarlay = new QHBoxLayout();
    LoadToolBar(Toolbarlay);
    Toolbarlay->setAlignment(Qt::AlignLeft);

    programmer = new zprog(this);

    main_lay->addLayout(Toolbarlay);
    main_lay->addWidget(programmer);


    QWidget *widget = new QWidget();
    widget->setLayout(main_lay);
    setCentralWidget(widget);
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

void QuickProg::LoadSetting()
{
    bool OK;
    QSettings app("config.ini", QSettings::IniFormat);
    AppSetting.EnablePowerControl = app.value("PowerControl",false).toBool();
    AppSetting.IsPowerControlInverse = app.value("PowerControlInverse",false).toBool();
    AppSetting.PowerControlPin = app.value("PowerControlPin",0).toUInt(&OK)==1 ? RTS_pin:DTR_pin;

    AppSetting.EnableUSBFilter = app.value("USBFilter",false).toBool();
    AppSetting.USB_PID =(uint16_t) app.value("PID",0).toUInt(&OK);
    AppSetting.USB_VID =(uint16_t) app.value("VID",0).toUInt(&OK);
}

void QuickProg::SaveSetting()
{
    QSettings app("config.ini", QSettings::IniFormat);
    app.setValue("PowerControl",QVariant::fromValue(AppSetting.EnablePowerControl));
    app.setValue("PowerControlInverse",QVariant::fromValue(AppSetting.IsPowerControlInverse));
    app.setValue("PowerControlPin",QVariant::fromValue(AppSetting.PowerControlPin));

    app.setValue("USBFilter",QVariant::fromValue(AppSetting.EnableUSBFilter));
    app.setValue("PID",QVariant::fromValue(AppSetting.USB_PID));
    app.setValue("VID",QVariant::fromValue(AppSetting.USB_VID));
}

void QuickProg::handel_SettingAction()
{
    settingdialog setting(&AppSetting,this);
    if(setting.exec())
    {
        /*save setting to file*/
        SaveSetting();

        /*try load setting*/

    }
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

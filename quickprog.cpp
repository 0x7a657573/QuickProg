#include "quickprog.h"
#include "ui_quickprog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QFileDialog>
#include <QProcess>

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

    main_lay->addLayout(Toolbarlay);
    LoadProgrammer(main_lay);


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
    xPort = new QComboBox();
    xPort->setFixedHeight(30);
    xPort->setToolTip(tr("Select Serial Port"));

    /*Baud Rate*/
    int Bauds[] = {115200,230400,460800,921600};
    xBaud = new QComboBox(this);
    xBaud->setFixedHeight(30);
    xBaud->setToolTip(tr("set BaudRate"));
    for(long unsigned int i=0;i<(sizeof(Bauds)/sizeof(int));i++)
    {
       xBaud->addItem( QString::number(Bauds[i]) );
    }
    xBaud->setCurrentIndex(0); /*Set in 115200*/

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

    if(!AppSetting.EnableUSBFilter)
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
    //qDebug() << app.value("PowerControlPin",0).toUInt(&OK);
    AppSetting.PowerControlPin = (app.value("PowerControlPin",false).toBool()) ? DTR_pin:RTS_pin;

    AppSetting.EnableUSBFilter = app.value("USBFilter",false).toBool();
    AppSetting.USB_PID =(uint16_t) app.value("PID",0).toUInt(&OK);
    AppSetting.USB_VID =(uint16_t) app.value("VID",0).toUInt(&OK);
    AppSetting.USB_row =(uint8_t) app.value("ROW",0).toUInt(&OK);
    AppSetting.USB_col =(uint8_t) app.value("COL",0).toUInt(&OK);
}

void QuickProg::SaveSetting()
{
    QSettings app("config.ini", QSettings::IniFormat);
    app.setValue("PowerControl",QVariant::fromValue(AppSetting.EnablePowerControl));
    app.setValue("PowerControlInverse",QVariant::fromValue(AppSetting.IsPowerControlInverse));
    bool pci = (AppSetting.PowerControlPin==DTR_pin);
    app.setValue("PowerControlPin",QVariant::fromValue(pci));

    app.setValue("USBFilter",QVariant::fromValue(AppSetting.EnableUSBFilter));
    app.setValue("PID",QVariant::fromValue(AppSetting.USB_PID));
    app.setValue("VID",QVariant::fromValue(AppSetting.USB_VID));
    app.setValue("ROW",QVariant::fromValue(AppSetting.USB_row));
    app.setValue("COL",QVariant::fromValue(AppSetting.USB_col));
    app.sync();
}

void QuickProg::LoadProgrammer(QVBoxLayout *parentlay)
{
    if(AppSetting.EnableUSBFilter)
    {
        for(int y=0;y<AppSetting.USB_row;y++)
        {
            QHBoxLayout *Row_lay = new QHBoxLayout();
            for(int x=0;x<AppSetting.USB_col;x++)
            {

                zprog *Tp = new zprog(this);
                Tp->setEnabled(false);
                programmers.append(Tp);
                Row_lay->addWidget(Tp);
            }
            parentlay->addLayout(Row_lay);
        }

        int index=0;
        Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts())
        {
            if((AppSetting.USB_PID==0 || AppSetting.USB_PID==port.productIdentifier()) &&
              (AppSetting.USB_VID==0 || AppSetting.USB_VID==port.vendorIdentifier()))
            {
                if(index<programmers.length())
                {
                    programmers[index]->setEnabled(true);
                    programmers[index]->SetSerialPort(port.portName());
                    index++;
                }
            }
        }
    }
    else
    {
        zprog *Tp = new zprog(this);
        programmers.append(Tp);
        parentlay->addWidget(Tp);
    }
}

void QuickProg::handel_SettingAction()
{
    settingdialog setting(&AppSetting,this);
    if(setting.exec())
    {
        /*save setting to file*/
        SaveSetting();

        /*try load setting*/
        qApp->quit();
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    }
}

void QuickProg::handel_BrowseFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"));
    if(fileName != "")
    {
        LPath->setText(fileName);
    }
}

void QuickProg::handel_StartAction()
{
      Q_FOREACH(zprog *programmer, programmers)
      {
        if(programmer->isEnabled())
        {
            if(!AppSetting.EnableUSBFilter)
                programmer->SetSerialPort(xPort->currentText());
            programmer->SetPowerContorl(AppSetting.EnablePowerControl,
                                        AppSetting.PowerControlPin==DTR_pin,
                                        AppSetting.IsPowerControlInverse);
            programmer->SetfirmwarePath(LPath->text());
            programmer->start();
        }
      }

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

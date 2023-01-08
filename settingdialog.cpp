#include "settingdialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

settingdialog::settingdialog(AppSetting_t *setting,QWidget *parent) : QDialog(parent)
{
    app = setting;

    Mainlayout = new QVBoxLayout(this);

    /*Power Control Section*/
    gBox = new QGroupBox();
    gBox->setCheckable(true);
    gBox->setChecked(app->EnablePowerControl);
    gBox->setTitle("Power Control");

    QVBoxLayout *PowerControl_main = new QVBoxLayout(gBox);
    QHBoxLayout *PowerControl_lay = new QHBoxLayout();
    PowerControl_main->addLayout(PowerControl_lay);

    QLabel *PowerControl_lb = new QLabel(tr("Pin Name"),gBox);
    PowerControl_cb = new QComboBox(gBox);
    PowerControl_cb->addItem(tr("DTR"));
    PowerControl_cb->addItem(tr("RTS"));
    if(app->PowerControlPin==RTS_pin)
        PowerControl_cb->setCurrentIndex(1);

    PowerControl_ck = new QCheckBox(gBox);
    PowerControl_ck->setText(tr("Inverted"));
    PowerControl_ck->setCheckState(app->IsPowerControlInverse ? Qt::CheckState::Checked:Qt::CheckState::Unchecked);
    PowerControl_main->addWidget(PowerControl_ck);

    PowerControl_lay->addWidget(PowerControl_lb);
    PowerControl_lay->addWidget(PowerControl_cb);    


    /*Usb Scanner Section*/
    uBox = new QGroupBox();
    QHBoxLayout *UsbScanner_main = new QHBoxLayout(uBox);
    uBox->setCheckable(true);
    uBox->setChecked(app->EnableUSBFilter);
    uBox->setTitle("Usb Scanner");

    eVID = new QLineEdit(tr("0x") + QString("%1").arg(app->USB_VID,4,16,QLatin1Char('0')).toUpper());
    ePID = new QLineEdit(tr("0x") + QString("%1").arg(app->USB_PID,4,16,QLatin1Char('0')).toUpper());

    UsbScanner_main->addWidget(new QLabel(tr("VID"),uBox));
    UsbScanner_main->addWidget(eVID);
    UsbScanner_main->addWidget(new QLabel(tr("PID"),uBox));
    UsbScanner_main->addWidget(ePID);

    uBox->setLayout(UsbScanner_main);


    /*Button Section*/
    QHBoxLayout *PushButton_lay = new QHBoxLayout();
    QPushButton *Cancel_btn = new QPushButton("Cancel");
    QPushButton *Ok_btn = new QPushButton("Ok");
    connect(Cancel_btn,SIGNAL(clicked()),this,SLOT(handel_cancel()));
    connect(Ok_btn,SIGNAL(clicked()),this,SLOT(handel_save()));

    PushButton_lay->addWidget(Cancel_btn);
    PushButton_lay->addWidget(Ok_btn);

    Mainlayout->addWidget(gBox);
    Mainlayout->addWidget(uBox);
    Mainlayout->addLayout(PushButton_lay);
}

void settingdialog::handel_save()
{
    app->EnablePowerControl = gBox->isChecked();
    app->IsPowerControlInverse = PowerControl_ck->isChecked();
    app->PowerControlPin = PowerControl_cb->currentIndex()==0 ? DTR_pin:RTS_pin;

    bool ok;
    app->EnableUSBFilter = uBox->isChecked();
    app->USB_PID = (uint16_t)ePID->text().toInt(&ok,16);
    app->USB_VID = (uint16_t)eVID->text().toInt(&ok,16);
    this->accept();
}

void settingdialog::handel_cancel()
{
    this->reject();
}

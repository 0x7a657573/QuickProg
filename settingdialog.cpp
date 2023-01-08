#include "settingdialog.h"
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

settingdialog::settingdialog(QWidget *parent) : QDialog(parent)
{
    Mainlayout = new QVBoxLayout(this);

    /*Power Control Section*/
    QGroupBox *gBox = new QGroupBox();
    gBox->setCheckable(true);
    gBox->setChecked(false);
    gBox->setTitle("Power Control");

    QVBoxLayout *PowerControl_main = new QVBoxLayout(gBox);
    QHBoxLayout *PowerControl_lay = new QHBoxLayout();
    PowerControl_main->addLayout(PowerControl_lay);

    QLabel *PowerControl_lb = new QLabel(tr("Pin Name"),gBox);
    QComboBox *PowerControl_cb = new QComboBox(gBox);
    PowerControl_cb->addItem(tr("DTR"));
    PowerControl_cb->addItem(tr("RTS"));

    QCheckBox *PowerControl_ck = new QCheckBox(gBox);
    PowerControl_ck->setText(tr("Inverted"));
    PowerControl_main->addWidget(PowerControl_ck);

    PowerControl_lay->addWidget(PowerControl_lb);
    PowerControl_lay->addWidget(PowerControl_cb);
    //gBox->setLayout(PowerControl_main);


    /*Usb Scanner Section*/
    QGroupBox *uBox = new QGroupBox();
    QHBoxLayout *UsbScanner_main = new QHBoxLayout(uBox);
    uBox->setCheckable(true);
    uBox->setChecked(false);
    uBox->setTitle("Usb Scanner");

    QLineEdit *eVID = new QLineEdit();
    QLineEdit *ePID = new QLineEdit();


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

    //this->layout()->addItem(Mainlayout);
}

void settingdialog::handel_save()
{
    this->accept();
}

void settingdialog::handel_cancel()
{
    this->reject();
}

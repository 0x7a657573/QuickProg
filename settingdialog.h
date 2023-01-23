#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H
#include <QVBoxLayout>
#include <QCheckBox>
#include <QDialog>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>

typedef enum
{
    DTR_pin = 1,
    RTS_pin
}PowerPin_t;

typedef struct
{
    bool EnablePowerControl;
    bool IsPowerControlInverse;
    bool PowerOffAfterProgramm;
    PowerPin_t PowerControlPin;
    bool EnableUSBFilter;
    uint16_t USB_VID;
    uint16_t USB_PID;
    uint8_t  USB_col;
    uint8_t  USB_row;
}AppSetting_t;

class settingdialog : public QDialog
{
    Q_OBJECT
public:
    settingdialog(AppSetting_t *setting,QWidget *parent = nullptr);

private slots:
    void handel_cancel();
    void handel_save();

private:
    QVBoxLayout *Mainlayout;

protected:
    QGroupBox *gBox;
    QGroupBox *uBox;
    QComboBox *PowerControl_cb;
    QCheckBox *PowerControl_ck;
    QCheckBox *PowerControl_po;
    QLineEdit *eVID;
    QLineEdit *ePID;
    QLineEdit *eRow;
    QLineEdit *eCol;
    AppSetting_t *app;
};

#endif // SETTINGDIALOG_H

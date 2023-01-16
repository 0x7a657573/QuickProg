#ifndef QUICKPROG_H
#define QUICKPROG_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QLineEdit>
#include <zprog/zprog.h>
#include <QSettings>
#include <settingdialog.h>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class QuickProg; }
QT_END_NAMESPACE

class QuickProg : public QMainWindow
{
    Q_OBJECT

public:
    QuickProg(QWidget *parent = nullptr);
    ~QuickProg();

private slots:
    void handel_SettingAction();
    void handel_LoadSerialPort();
    void handel_TaskEndAction(int id);
    void handel_StartAction();
    void handel_BrowseFile();
    void handel_AboutMe();

private:
    QComboBox *xPort;
    QComboBox *xBaud;
    QLineEdit *LPath;
    QPushButton *btnStart;
    Ui::QuickProg *ui;
    void LoadToolBar(QHBoxLayout *lay);
    void LoadSetting();
    void SaveSetting();
    void LoadProgrammer(QVBoxLayout *parentlay);

protected:
    QList<bool> programmerStatus;
    QList<zprog*> programmers;
    AppSetting_t AppSetting;
};
#endif // QUICKPROG_H

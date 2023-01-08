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
    void handel_StartAction();
    void handel_BrowseFile();

private:
    QComboBox *xPort;
    QComboBox *xBaud;
    QLineEdit *LPath;
    Ui::QuickProg *ui;
    void LoadToolBar(QHBoxLayout *lay);

protected:
    zprog *programmer;
    QSettings Settings;
};
#endif // QUICKPROG_H

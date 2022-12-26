#ifndef QUICKPROG_H
#define QUICKPROG_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <zprog/zprog.h>

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
    void handel_LoadSerialPort();
    void handel_StartAction();

private:
    QComboBox *xPort;
    QComboBox *xBaud;

    Ui::QuickProg *ui;
    void LoadToolBar(QHBoxLayout *lay);

protected:
    zprog *programmer;
};
#endif // QUICKPROG_H
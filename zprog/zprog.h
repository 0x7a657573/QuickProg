#ifndef ZPROG_H
#define ZPROG_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QSerialPort>
#include <QThread>
#include <QLabel>

class zprog : public QWidget
{
    Q_OBJECT
public:
    explicit zprog(int id=0,QWidget *parent = nullptr);
    void SetBaud(int baud);
    void SetSerialPort(const QString PortName);
    void SetfirmwarePath(const QString firmwarePath);
    void SetPowerContorl(bool Enable,bool Is_DTR,bool Is_Not=false);
    void start(void);

signals:
    void operate(const QString &);
    void ended(int id);

private slots:
    void setProgress(int value);
    void writelog(QString str);
    void finished(bool hasError);

private:
    void log();
    void DoPrograming(void);

protected:
    int xid;
    bool PowerContorl;
    bool IsPcDTR;
    bool IsPcNot;

    QString xFirmwarePath;
    QString xPortName;

    /*Layouts*/
    QVBoxLayout *Layout;
    QHBoxLayout *HLay;

    /*log window*/
    QPlainTextEdit *log_view;

    /*ProgressBar*/
    QProgressBar *Progress;

    /*status*/
     QLabel *imgStatus;

     int xBaud;
};

#endif // ZPROG_H

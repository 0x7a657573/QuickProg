#ifndef ZPROG_H
#define ZPROG_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QSerialPort>
#include <QThread>

class zprog : public QWidget
{
    Q_OBJECT
public:
    explicit zprog(QWidget *parent = nullptr);
    void SetSerialPort(const QString PortName);
    void start(void);

signals:


private slots:
    void setProgress(int value);

private:
    void log();
    void DoPrograming(void);
protected:

    QSerialPort SerialPort;
    /*Layouts*/
    QVBoxLayout *Layout;

    /*log window*/
    QPlainTextEdit *log_view;

    /*ProgressBar*/
    QProgressBar *Progress;
};

#endif // ZPROG_H

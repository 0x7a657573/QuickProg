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
    void operate(const QString &);

private slots:
    void setProgress(int value);
    void writelog(QString str);

private:
    void log();
    void DoPrograming(void);
protected:

    QString xPortName;

    /*Layouts*/
    QVBoxLayout *Layout;

    /*log window*/
    QPlainTextEdit *log_view;

    /*ProgressBar*/
    QProgressBar *Progress;
};

#endif // ZPROG_H

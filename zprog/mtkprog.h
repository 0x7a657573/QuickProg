#ifndef MTKPROG_H
#define MTKPROG_H
#include <QObject>
#include <QString>
#include <QSerialPort>


/*cmd general define--------------------------*/
#define CMD_READ_16 0xA2
#define CMD_READ16 0xD0
#define CMD_READ32 0xD1
#define CMD_WRITE16 0xD2
#define CMD_WRITE32 0xD4
#define CMD_JUMP_DA 0xD5
#define CMD_SEND_DA 0xD7
#define CMD_SEND_EPP 0xD9
/*end cmd general define--------------------------*/



class mtkprog : public QObject
{
    Q_OBJECT

public:
    mtkprog(QString PortName);

    /*type*/
    typedef struct
    {
        uint16_t CPU_HW;
        uint16_t CPU_SW;
        uint16_t CPU_ID;
        uint16_t CPU_SB;
    }cpu_t;
    cpu_t Core;

public slots:
    void Start(void);

signals:
    void progress(int percent);
    void wlog(QString str);
    void finished();
    void error(QString err);

private:
    QString xPort_PortName;
    QSerialPort *xPort;
    QString loadBootLoader;

 protected:
    uint32_t ReadTimeout;
    void die(void);
    bool open(void);
    QByteArray send(QByteArray &data,uint32_t sz=0);
    QByteArray cmd(QByteArray &command, uint32_t sz=0);
    uint16_t da_read_16(uint32_t addr,uint32_t sz=1);
    bool da_write16(uint32_t addr,uint16_t val);
    bool da_write32(uint32_t addr,uint32_t val);
    bool connect(uint32_t timeout=30);
};

#endif // MTKPROG_H

#ifndef MTKPROG_H
#define MTKPROG_H
#include <QObject>
#include <QString>
#include <QSerialPort>

/*DA define-----------------------------------*/
#define DA_SYNC 0xC0
#define DA_FORMAT_FAT 0xB8
#define DA_CONFIG_EMI 0xD0
#define DA_POST_PROCESS 0xD1
#define DA_SPEED 0xD2
#define DA_MEM 0xD3
#define DA_FORMAT 0xD4
#define DA_WRITE 0xD5
#define DA_READ 0xD6
#define DA_WRITE_REG16 0xD7
#define DA_READ_REG16 0xD8
#define DA_FINISH 0xD9
#define DA_GET_DSP_VER 0xDA
#define DA_ENABLE_WATCHDOG 0xDB
#define DA_NFB_WRITE_BLOADER 0xDC
#define DA_NAND_IMAGE_LIST 0xDD
#define DA_NFB_WRITE_IMAGE 0xDE
#define DA_NAND_READPAGE 0xDF
/*end DA define-----------------------------------*/

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
#define CONF 0x69
#define STOP 0x96
#define ACK 0x5A
#define NACK 0xA5


typedef struct
{
    uint32_t offset;
    uint32_t size;
    uint32_t address;
}DA_t;


class mtkprog : public QObject
{
    Q_OBJECT

public:
    mtkprog(QString PortName,QString firmware);

    /*type*/
    typedef struct
    {
        uint16_t CPU_HW;
        uint16_t CPU_SW;
        uint16_t CPU_ID;
        uint16_t CPU_SB;
    }cpu_t;
    cpu_t Core;

    typedef enum {
        UART_BAUD_921600=0x01,
        UART_BAUD_460800=0x02,
        UART_BAUD_230400=0x03,
        UART_BAUD_115200=0x04,
    }mtk_baud;

public slots:
    void Start(void);

signals:
    void progress(int percent);
    void wlog(QString str);
    void finished();
    void error(QString err);

private:
    QString xPort_PortName;
    QString xFirmware_Path;

    QSerialPort *xPort;
    QByteArray DaFile;
    QByteArray Dafirmware;
    uint32_t max_pg;
    uint32_t cur_pg;
    void setup_progress(uint32_t max);
    void update_progress(uint32_t val);

 protected:
    mtk_baud BaudRate;
    uint32_t ReadTimeout;
    void die(void);
    bool open(void);
    QByteArray send(QByteArray &data,uint32_t sz=0);
    QByteArray cmd(QByteArray &command, uint32_t sz=0);
    uint16_t da_read_16(uint32_t addr,uint32_t sz=1);
    bool da_write16(uint32_t addr,uint16_t val);
    bool da_write32(uint32_t addr,uint32_t val);
    bool loadBootLoader(QString fname);
    QByteArray get_da(uint32_t offset,uint32_t size);
    bool da_send_da(uint32_t address,uint32_t size,QByteArray &data,uint32_t block=4096);
    bool sendFlashInfo(uint32_t offset);
    bool da_changebaud(mtk_baud baud=UART_BAUD_460800);
    bool uploadApplication();
    bool loadfirmware();

    bool connect(uint32_t timeout=30);
    bool da_start(void);
};

#endif // MTKPROG_H

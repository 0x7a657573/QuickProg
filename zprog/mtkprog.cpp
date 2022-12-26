#include "mtkprog.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>

mtkprog::mtkprog(QString PortName)
{
    xPort_PortName = PortName;
    xPort = nullptr;
    ReadTimeout = 100;
}

bool mtkprog::open(void)
{
    if(xPort==nullptr)  return false;

    xPort->setBaudRate(115200);
    /*open Serial Port*/
    if(xPort->open(QIODevice::ReadWrite)==true)
    {
        emit wlog(QString("Connected to {%1}").arg(xPort->portName()));
    }
    else
    {
        emit wlog(xPort->errorString());
        emit finished();
        return false;
    }
    return true;
}

void mtkprog::die(void)
{
    if(xPort!=nullptr)
    {
        xPort->close();
        delete(xPort);
    }
    emit finished();
}

QByteArray mtkprog::send(QByteArray &data,uint32_t sz)
{
    if(data.size())
    {
        xPort->write(data);
        xPort->waitForBytesWritten(100);
    }
    QByteArray rbyte;
    rbyte.clear();
    if(sz)
    {
        uint32_t Try = ReadTimeout;
        while(sz - rbyte.size() && Try--)
        {
            xPort->waitForReadyRead(10);
            rbyte += xPort->read(sz - rbyte.size());
            //qDebug() << "WeRead " << rbyte.size() << "wait for" << sz - rbyte.size() << "byte";
        }
        //qDebug() << "send R" << sz << rbyte.toHex(',');
    }
    return rbyte;
}

QByteArray mtkprog::cmd(QByteArray &command, uint32_t sz)
{
    QByteArray sbuf;

    if (command.size())
    {
        sbuf = send(command,command.size());
    }

    if (sz)
    {
        uint32_t Try = ReadTimeout;
        sbuf.clear();
        while(sz - sbuf.size() && Try--)
        {
            xPort->waitForReadyRead(10);
            sbuf += xPort->read(sz - sbuf.size());
        }
    }

    return sbuf;
}

uint16_t mtkprog::da_read_16(uint32_t addr,uint32_t sz)
{
    QByteArray data;

    data.append(CMD_READ_16);
    data.append((uint8_t)((addr >> 24) & 0xFF));
    data.append((uint8_t)((addr >> 16) & 0xFF));
    data.append((uint8_t)((addr >> 8) & 0xFF));
    data.append((uint8_t)((addr)&0xFF));
    data.append((uint8_t)((sz >> 24) & 0xFF));
    data.append((uint8_t)((sz >> 16) & 0xFF));
    data.append((uint8_t)((sz >> 8) & 0xFF));
    data.append((uint8_t)((sz)&0xFF));
    //qDebug() << "cmd" << data.toHex(',');

    QByteArray rp = cmd(data, sz * 2);
    if(rp.size()==2)
    {
        return (rp[0]<<8) + rp[1];
    }
    return 0;
}

bool mtkprog::da_write16(uint32_t addr,uint16_t val)
{
    uint32_t TMP = 1;
    QByteArray AdrCmd;
    AdrCmd.append(CMD_WRITE16);
    AdrCmd.append((uint8_t)((addr >> 24) & 0xFF));
    AdrCmd.append((uint8_t)((addr >> 16) & 0xFF));
    AdrCmd.append((uint8_t)((addr >> 8) & 0xFF));
    AdrCmd.append((uint8_t)((addr)&0xFF));
    AdrCmd.append((uint8_t)((TMP >> 24) & 0xFF));
    AdrCmd.append((uint8_t)((TMP >> 16) & 0xFF));
    AdrCmd.append((uint8_t)((TMP >> 8) & 0xFF));
    AdrCmd.append((uint8_t)((TMP)&0xFF));

    QByteArray Read = cmd(AdrCmd, 2);
    if(Read.indexOf(QByteArray("\x00\x01"))!=0 || Read.size()!=2 )
    {
        emit wlog(QString("W16 Can not set Adr %1").arg(QString::number(addr,16)));
        return false;
    }

    QByteArray Val;
    Val.append((uint8_t)((val >> 8) & 0xFF));
    Val.append((uint8_t)(val & 0xFF));

    Read = cmd(Val, 2);
    if(Read.indexOf(QByteArray("\x00\x01"))!=0 || Read.size()!=2 )
    {
        emit wlog(QString("W16 Can not set val %1").arg(QString::number(val,16)));
        return false;
    }
    return true;
}

bool mtkprog::da_write32(uint32_t addr,uint32_t val)
{
    uint32_t TMP = 1;
    QByteArray AdrCmd;
    AdrCmd.append(CMD_WRITE32);
    AdrCmd.append((uint8_t)((addr >> 24) & 0xFF));
    AdrCmd.append((uint8_t)((addr >> 16) & 0xFF));
    AdrCmd.append((uint8_t)((addr >> 8) & 0xFF));
    AdrCmd.append((uint8_t)((addr)&0xFF));
    AdrCmd.append((uint8_t)((TMP >> 24) & 0xFF));
    AdrCmd.append((uint8_t)((TMP >> 16) & 0xFF));
    AdrCmd.append((uint8_t)((TMP >> 8) & 0xFF));
    AdrCmd.append((uint8_t)((TMP)&0xFF));

    QByteArray Read = cmd(AdrCmd, 2);
    if(Read.indexOf(QByteArray("\x00\x01"))!=0 || Read.size()!=2 )
    {
        emit wlog(QString("W32 Can not set Adr %1").arg(QString::number(addr,16)));
        return false;
    }

    QByteArray Val;
    Val.append((uint8_t)((val >> 24) & 0xFF));
    Val.append((uint8_t)((val >> 16) & 0xFF));
    Val.append((uint8_t)((val >> 8) & 0xFF));
    Val.append((uint8_t)(val & 0xFF));

    Read = cmd(Val, 2);
    if(Read.indexOf(QByteArray("\x00\x01"))!=0 || Read.size()!=2 )
    {
        emit wlog(QString("W16 Can not set val %1").arg(QString::number(val,16)));
        return false;
    }
    return true;
}


bool mtkprog::connect(uint32_t timeout)
{
    QElapsedTimer elapsed_timer;
    emit wlog(tr("Please reset the device.\nWaiting."));
    elapsed_timer.start();
    const char hA = 0xA0;
    const char rID[] = {0x0A,0x50,0x05};
    while(true)
    {
        xPort->readAll();
        xPort->write(&hA,1);
        xPort->waitForReadyRead(50);
        QByteArray data = xPort->readAll();
        if(data.indexOf(0x5F)!=-1)
        {
            data.clear();
            xPort->readAll();
            xPort->write(rID,sizeof(rID));
            xPort->flush();
            xPort->waitForBytesWritten(20);
            xPort->waitForReadyRead(50);
            QByteArray CorectReplay("\xF5\xAF\xFA",3);
            data = xPort->readAll();
            if(data.indexOf(CorectReplay)!=-1)
            {
                //qDebug() << data.toHex(',');
                emit wlog(tr("Connect to Device :)"));
                break;
            }
            else
            {
                emit wlog(tr("Boot Erorr!"));
                emit wlog(QString("R: {%1}").arg(data.toHex(',')));
                return false;
            }
        }

        if (elapsed_timer.elapsed() > timeout*1000)
        {
            emit wlog(tr("TimeOut :("));
            return false;
        }
    }

    xPort->readAll();
    Core.CPU_HW = da_read_16(0x80000000);  // CPU_HW = CB01
    Core.CPU_SW = da_read_16(0x80000004);  // CPU_SW = 0001
    Core.CPU_ID = da_read_16(0x80000008);  // CPU_ID = 6261
    Core.CPU_SB = da_read_16(0x8000000C);  // CPU_SB = 8000
    emit wlog(QString("CPU ID: %1").arg(QString::number(Core.CPU_ID,16)));
    if(!da_write16(0xa0700a28, 0x4010)) return false;  // 01
    if(!da_write16(0xa0700a00, 0xF210)) return false;  // 02
    if(!da_write16(0xa0030000, 0x2200)) return false;  // 03
    if(!da_write16(0xa071004c, 0x1a57)) return false;  // 04
    if(!da_write16(0xa071004c, 0x2b68)) return false;  // 05
    if(!da_write16(0xa071004c, 0x042e)) return false;  // 06
    if(!da_write16(0xa0710068, 0x586a)) return false;  // 07
    if(!da_write16(0xa0710074, 0x0001)) return false;  // 08
    if(!da_write16(0xa0710068, 0x9136)) return false;  // 09
    if(!da_write16(0xa0710074, 0x0001)) return false;  // 10
    if(!da_write16(0xa0710000, 0x430e)) return false;  // 11
    if(!da_write16(0xa0710074, 0x0001)) return false;  // 12
    if(!da_write32(0xa0510000, 0x00000002)) return false;  // ???

    if(Core.CPU_ID == 0x6261)
    {
        loadBootLoader = "mt6261_da.bin";
    }
    else
    {
        emit wlog(QString("QuickProg does not support this SoC: %1").arg(QString::number(Core.CPU_ID,16)));
        return false;
    }

    return true;
}

void mtkprog::Start(void)
{
    xPort = new QSerialPort(xPort_PortName);
    if(!open())
    {
        emit wlog(QString("Can not open Port {%1}").arg(xPort_PortName));
        die();
        return;
    }
    if(!connect())
    {
        emit wlog(QString("Can not Connect to device!"));
        die();
        return;
    }

    for(int i=0;i<101;i++)
    {
        xPort->write("Fuck You");
        emit progress(i);
        QThread::msleep(50);
    }

    die();
}

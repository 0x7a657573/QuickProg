#include "mtkprog.h"
#include <QDebug>
#include <QThread>
#include <QElapsedTimer>
#include <QFile>
#include <QtEndian>

DA_t MT6261_DA[] =
{
    {.offset = 0x00000, .size=0x00718, .address=0x70007000},
    {.offset = 0x00718, .size=0x1e5c8, .address=0x10020000}
};

mtkprog::mtkprog(QString PortName,QString firmware, bool PwCo,bool PwIsDTR,bool PwInverse)
{
    xFirmware_Path = firmware;
    xPort_PortName = PortName;
    xPort = nullptr;
    ReadTimeout = 200;
    BaudRate = UART_BAUD_460800;

    mtk_PwCo = PwCo;
    mtk_PwIsDTR = PwIsDTR;
    mtk_PwInverse = PwInverse;
    status = mtk_stop;
}

void mtkprog::setup_progress(uint32_t max)
{
    max_pg = max;
    cur_pg = 0;
    emit progress(0);
}

void mtkprog::update_progress(uint32_t val)
{
    cur_pg += val;
    float p = cur_pg * 100;
    p /= max_pg;
    //qDebug() << p << cur_pg << max_pg;
    if(p>100.0f)
        p = 100;
    emit progress(p);
}

void mtkprog::PowerControl(bool status)
{
    if(mtk_PwCo==false)
            return;

    if(xPort==nullptr)
        return;


    if(mtk_PwIsDTR)
    {
        xPort->setDataTerminalReady(status!=mtk_PwInverse);
    }
    else
    {
        xPort->setRequestToSend(status!=mtk_PwInverse);
    }
}

bool mtkprog::open(void)
{
    if(xPort==nullptr)  return false;

    xPort->setBaudRate(115200);
    /*open Serial Port*/
    if(xPort->open(QIODevice::ReadWrite)==true)
    {
        PowerControl(false);// Power Off Device
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

void mtkprog::die(bool HasError)
{
    if(xPort!=nullptr)
    {
        xPort->close();
        delete(xPort);
    }

    if(HasError==false)
        status = mtk_finished;
    else
        status = mtk_finishedWithError;

    emit finishedWithStatus(HasError);
    emit finished();
}

QByteArray mtkprog::send(QByteArray &data,uint32_t sz)
{
    if(data.size())
    {
        xPort->write(data);
        xPort->waitForBytesWritten(500);
    }
    QByteArray rbyte;
    rbyte.clear();
    if(sz)
    {
        int32_t Try = ReadTimeout;
        while(sz - rbyte.size() && Try--)
        {
            xPort->waitForReadyRead(10);
            rbyte += xPort->read(sz - rbyte.size());
            //qDebug() << "WeRead " << rbyte.size() << "wait for" << sz - rbyte.size() << "byte";
        }
//        if(Try<=0)
//            qDebug() << "Wait TimeOut !";

        //qDebug() << "send R" << sz << rbyte.toHex(',');
    }

    if(sz != rbyte.size())
    {
        qDebug() << "We Not Get Enogh Byte :(((";
        rbyte.fill('!',sz);
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

bool mtkprog::loadBootLoader(QString fname)
{
    DaFile.clear();
    QFile f(fname);
    if(!f.open(QIODevice::ReadOnly))
    {
        //qDebug()<<"filenot opened";
        return false;
    }

    DaFile = f.readAll();
    f.close();
    //qDebug() << "DA read " << DaFile.size() << "bytes";
    return true;
}

bool mtkprog::connect(uint32_t timeout)
{
    QElapsedTimer elapsed_timer;
    QElapsedTimer Power_timer;
    emit wlog(tr("Please reset the device.\nWaiting."));
    elapsed_timer.start();
    Power_timer.start();
    const char hA = 0xA0;
    const char rID[] = {0x0A,0x50,0x05};
    QThread::sleep(1);
    PowerControl(true);
    while(true)
    {
        //xPort->readAll();
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

        if (Power_timer.elapsed() > 3*1000)
        {
            Power_timer.restart();
            emit wlog(tr("Power Action"));
            PowerControl(false);
            QThread::sleep(1);
            xPort->flush();
            xPort->readAll();
            PowerControl(true);
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
        loadBootLoader(":/bin/MT6261_Bootloader");
    }
    else
    {
        emit wlog(QString("QuickProg does not support this SoC: %1").arg(QString::number(Core.CPU_ID,16)));
        return false;
    }

    return true;
}

QByteArray mtkprog::get_da(uint32_t offset,uint32_t size)
{
    QByteArray Select = DaFile.mid(offset,size);
    return Select;
}

bool mtkprog::da_send_da(uint32_t address,uint32_t size,QByteArray &data,uint32_t block)
{
    QByteArray dCmd;
    dCmd.append(CMD_SEND_DA);
    dCmd.append((uint8_t)((address >> 24) & 0xFF));
    dCmd.append((uint8_t)((address >> 16) & 0xFF));
    dCmd.append((uint8_t)((address >> 8) & 0xFF));
    dCmd.append((uint8_t)((address)&0xFF));
    dCmd.append((uint8_t)((size >> 24) & 0xFF));
    dCmd.append((uint8_t)((size >> 16) & 0xFF));
    dCmd.append((uint8_t)((size >> 8) & 0xFF));
    dCmd.append((uint8_t)((size)&0xFF));
    dCmd.append((uint8_t)((block >> 24) & 0xFF));
    dCmd.append((uint8_t)((block >> 16) & 0xFF));
    dCmd.append((uint8_t)((block >> 8) & 0xFF));
    dCmd.append((uint8_t)((block)&0xFF));
    QByteArray res = cmd(dCmd, 2);

    if(res.indexOf(QByteArray("\x00\x00"))!=0 || res.size()!=2 )
    {
        emit wlog(QString("sDA Can not set Adr %1").arg(QString::number(address,16)));
        return false;
    }

    QByteArray NullCMD;
    while(data.size())
    {
        QByteArray send = data.mid(0,block);
        xPort->write(send);
        xPort->waitForBytesWritten(500);
        data = data.mid(block,-1);
        update_progress(send.size());

    }
    res = cmd(NullCMD,4); //checksum

    return true;
}

bool mtkprog::sendFlashInfo(uint32_t offset)
{
    QByteArray NullP;
    for(int i=0;i<512;i++)
    {
        QByteArray data = get_da(offset, 36);
        QByteArray check  = data.mid(0,4);
        if(check == QByteArray("\xFF\xFF\0\0"))
        {
            emit wlog(QString("Invalid flash info: %1").arg(data.mid(0,4).toHex(',')));
            return false;
        }
        offset += 36;
        QByteArray r = send(data, 1);

        if (r[0] == ACK)
        {
            r = cmd(NullP, 2);
            if(r.indexOf(QByteArray("\xA5\x69"))!=0 || r.size()!=2 )
            {
                emit wlog(QString("Flashinfo END: %1").arg(r.toHex(',')));
                return false;
            }
            return true;
        }
        if(r[0] != CONF)
        {
            emit wlog(QString("Flashinfo ACK Fail: %1").arg(r.toHex(',')));
            return false;
        }
    }
    return true;
}

bool mtkprog::da_start(void)
{
    emit wlog(QString("Load BootLoader"));

    setup_progress((uint32_t)(MT6261_DA[0].size + MT6261_DA[1].size));

    // SEND_DA_1
    uint32_t offset = MT6261_DA[0].offset;
    uint32_t size = MT6261_DA[0].size;
    uint32_t addr1 = MT6261_DA[0].address;
    QByteArray data = get_da(offset, size);
    emit wlog(QString("Upload DA Part 1 size %1").arg(size));
    da_send_da(addr1, size, data, 0x400);  // <--chs = D5AF.0000

    // SEND_DA_2
    offset = MT6261_DA[1].offset;
    size = MT6261_DA[1].size;
    uint32_t addr2 = MT6261_DA[1].address;
    data = get_da(offset, size);
    emit wlog(QString("Upload DA Part 2 size %1").arg(size));
    da_send_da(addr2, size, data, 0x800);  // <--chs = E423.0000

    offset += size;
    // CMD_JUMP_DA
    QByteArray Jd;
    Jd.append(CMD_JUMP_DA);
    Jd.append((uint8_t)((addr1 >> 24) & 0xFF));
    Jd.append((uint8_t)((addr1 >> 16) & 0xFF));
    Jd.append((uint8_t)((addr1 >> 8) & 0xFF));
    Jd.append((uint8_t)(addr1 & 0xFF));
    emit wlog(QString("Jump to DA offset 0x%1").arg(QString::number(addr1,16)));
    QByteArray jres = cmd(Jd, 2);
    if(jres.indexOf(QByteArray("\x00\x00"))!=0 || jres.size()!=2 )
    {
        emit wlog(QString("Err: Can not Jump To DA"));
        return false;
    }

    Jd.clear();
    // <-- C003028E DA_INFO: 0xC0 , Ver : 3.2 , BBID : 0x8E
    jres = cmd(Jd, 4);
    const uint8_t cf[] = {0xA5,0x05,0xFE,0x00,0x08,0x00,0x70,0x07,0xFF,0xFF,0x02,0x00,0x00,0x01,0x08};
    QByteArray f;
    for(unsigned int i=0;i<sizeof(cf);i++)
        f.append(cf[i]);
    QByteArray r = send(f, 1);
    if(r[0]!=ACK)
    {
        return false;
    }

    // FLASH ID INFOS
    emit wlog(QString("send flash info..."));
    if(!sendFlashInfo(offset))
    {
        emit wlog(QString("Flash info Error."));
        return false;
    }
    emit wlog(QString("Flash info OK."));

    QByteArray Setting;
    Setting.append((uint8_t)0);
    Setting.append((uint8_t)0);
    Setting.append((uint8_t)0);
    Setting.append((uint8_t)0);
    r = send(Setting, 256);  // EMI_SETTINGS ??
//    qDebug() << r.size() << r.toHex().toUpper();
//    if(r.size()!=256)
//    {
//        emit wlog(QString("Error EMI SETTINGS"));
//        return false;
//    }
    emit wlog(QString("Bootloadr Load ok."));
    return true;
}

bool mtkprog::da_changebaud(mtk_baud baud)
{
    uint32_t cBaud[] = {460800, 921600, 460800, 230400, 115200};
    if(baud==UART_BAUD_115200)
    {
        return true;
    }

    emit wlog(QString("Set Baud rate to %1").arg(cBaud[baud%5]));
    QByteArray sbaud;
    sbaud.append(DA_SPEED);
    sbaud.append((uint8_t)baud);
    sbaud.append(0x01);
    QByteArray r = send(sbaud,1);
    if(r[0] != ACK)
    {
        emit wlog(QString("Error set Baud rate!"));
        return false;
    }

    /*send ack*/
    QByteArray sACK;
    sACK.append(ACK);
    send(sACK);

    /*now change baud*/

    if(!xPort->setBaudRate(cBaud[baud%5]))
    {
        emit wlog(QString("Error to set Baud rate!"));
        return false;
    }
    QThread::msleep(200);

    /*try sync with module*/
    QByteArray sync;
    sync.append((uint8_t)DA_SYNC);
    for(int i=0;i<10;i++)
    {
        r = send(sync,1);
        if(r[0]==sync[0])
        {
            break;
        }
        QThread::msleep(200);
    }
    if(r[0]!=sync[0])
    {
        emit wlog(QString("Error to Sync with module"));
        return false;
    }

    r = send(sACK,1);
    if(r[0]!=ACK)
    {
        emit wlog(QString("module don't ACK Afer Change baud"));
        return false;
    }

    sync.clear();
    sync.append((uint8_t)0);
    for(int i=0;i<256;i++)
    {
        sync[0] = (uint8_t)i;
        r=send(sync,1);
        if(r[0]!=sync[0])
        {
            emit wlog(QString("Error in sync loop"));
            return false;
        }
    }

    emit wlog(QString("Set Baud OK."));
    return true;
}

bool mtkprog::loadfirmware()
{
    xFirmware.data.clear();

    QFile f(xFirmware_Path);
    if(!f.open(QIODevice::ReadOnly))
    {
        return false;
    }

    xFirmware.data = f.readAll();
    f.close();

    xFirmware.type = qFromLittleEndian<quint16>(xFirmware.data.mid(0x18,2));
    xFirmware.address = qFromLittleEndian<quint32>(xFirmware.data.mid(0x1C,4));
    xFirmware.Size = qFromLittleEndian<quint32>(xFirmware.data.mid(0x1C+4,4));

    if(xFirmware.data.length() != xFirmware.Size)
    {
        emit wlog("firmware Size mismatch");
        return false;
    }
    if(xFirmware.data.length() < 0x40)
    {
        emit wlog("firmware Invalid size.");
        return false;
    }

    /*check firmware*/
    QByteArray cHeader("MMM");
    QByteArray cInfo("FILE_INFO");

    if(xFirmware.data.mid(0,3) != cHeader)
    {
        emit wlog("firmware Invalid header 'MMM' expected.");
        return false;
    }

    if(xFirmware.data.mid(8,9) != cInfo)
    {
        emit wlog("firmware Invalid header 'FILE_INFO' expected.");
        return false;
    }

    emit wlog(QString("Firmware size %1 KByte").arg(xFirmware.data.size()/1024));
    return true;
}

bool mtkprog::da_mem(uint32_t address, uint32_t size,uint16_t  ftype,uint8_t file_count,uint8_t fota)
{
    emit wlog("da memory Format");

    QByteArray sendtemp;
    sendtemp.append((uint8_t)DA_MEM);
    sendtemp.append((uint8_t)fota);
    sendtemp.append((uint8_t)file_count);
    send(sendtemp, 0);

    uint32_t start_address = (address & 0x07FFFFFF);
    uint32_t end_address = start_address + size - 1;

    sendtemp.clear();
    sendtemp.append((uint8_t)((start_address >> 24) & 0xFF));
    sendtemp.append((uint8_t)((start_address >> 16) & 0xFF));
    sendtemp.append((uint8_t)((start_address >> 8) & 0xFF));
    sendtemp.append((uint8_t)((start_address)&0xFF));
    sendtemp.append((uint8_t)((end_address >> 24) & 0xFF));
    sendtemp.append((uint8_t)((end_address >> 16) & 0xFF));
    sendtemp.append((uint8_t)((end_address >> 8) & 0xFF));
    sendtemp.append((uint8_t)((end_address)&0xFF));
    sendtemp.append((uint8_t)(((uint32_t)size >> 24) & 0xFF));
    sendtemp.append((uint8_t)(((uint32_t)size >> 16) & 0xFF));
    sendtemp.append((uint8_t)(((uint32_t)size >> 8) & 0xFF));
    sendtemp.append((uint8_t)(((uint32_t)size) & 0xFF));
    QByteArray res = send(sendtemp,1);
    if(res.length()!=1 && res[0]!=ACK)
    {
        emit wlog("DA_MEM ACK");
        return false;
    }

    sendtemp.clear();
    res = send(sendtemp, 2); //filecount + ACK
    if(res[0] != file_count)
    {
        emit wlog("File count does not match");
        return false;
    }

    res = send(sendtemp, 4); // Format Ack Count for each file

    uint32_t format_ack_cnt = qFromBigEndian<quint32>(res);

    for (uint32_t i = 0; i < format_ack_cnt; i++)
    {
        res = send(sendtemp, 1);
        if (res[0] != ACK)
        {
            emit wlog("Error memory Format Failed");
            return false;
        }

    }

    res = send(sendtemp, 1);
    if (res[0] != ACK)
    {
        emit wlog("Error memory Format Failed");
        return false;
    }

    emit wlog("memory Formated");
    return true;
}

bool mtkprog::da_write(uint32_t block)
{

    QByteArray sendtemp;
    sendtemp.append((uint8_t)DA_WRITE);
    QByteArray res = send(sendtemp, 1);
    if(res[0] != ACK)
    {
        emit wlog("DA_WRITE ACK");
        return false;
    }

    // Sequential Erase (0x1). (0x0) for Best-Effort Erase, packet_length
    sendtemp.clear();
    sendtemp.append((uint8_t)0);
    sendtemp.append((uint8_t)((block >> 24) & 0xFF));
    sendtemp.append((uint8_t)((block >> 16) & 0xFF));
    sendtemp.append((uint8_t)((block >> 8) & 0xFF));
    sendtemp.append((uint8_t)((block)&0xFF));

    res = send(sendtemp, 2);
    if(!(res[0]==ACK && res[1]==ACK))
    {
        emit wlog("DA_WRITE Faild");
        return false;
    }

    emit wlog("da write ok");
    return true;
}

uint16_t mtkprog::crc_word(QByteArray data)
{
    uint32_t ch = 0;
    for (uint32_t i = 0; i < data.length(); i++)
    {
        ch += data[i] & 0xFF;
    }
    ch = ch & 0xFFFF;
    return ch;
}

bool mtkprog::da_write_data(QByteArray &fw_data,uint32_t block)
{
    setup_progress(fw_data.length());
    QByteArray data = fw_data;
    uint16_t cw = 0;
    while (data.length())
    {
        QByteArray xsend;
        xsend.append(ACK);
        xsend.append(data.mid(0,block));
        xPort->write(xsend);
        uint16_t crc = crc_word(data.mid(0,block));
        xPort->waitForBytesWritten();

        xsend.clear();
        xsend.append((uint8_t)((crc >> 8) & 0xFF));
        xsend.append((uint8_t)((crc)&0xFF));
        QByteArray ret = send(xsend, 1);
        if (ret[0] == CONF)
        {
            update_progress(block);
            cw += crc;
            data = data.mid(block,-1);
        }
        else if((uint8_t)ret[0] == (uint8_t)NACK)
        {
            qDebug() << "fuck";
            QElapsedTimer elapsed_timer;
            elapsed_timer.start();
            QByteArray empty;
            while(1)
            {
                QByteArray r = send(empty, 1);
                if (r[0] == ACK)
                {
                    empty.append(CONF);
                    xPort->write(empty);
                    xPort->waitForBytesWritten();
                    break;
                }

                if (elapsed_timer.elapsed() > 60*1000)
                {
                    emit wlog(tr("Firmware Data write timeout :("));
                    return false;
                }
            }
        }
        else
        {
            emit wlog(tr("Firmware fail"));
            return false;
        }
    }

    QElapsedTimer elapsed_timer;
    QByteArray empty;
    empty.clear();
    elapsed_timer.start();
    uint32_t ack_count = 0;
    while(1)
    {
        QByteArray r = send(empty, 1);
        if (r[0] == ACK)
        {
            ack_count += 1;
            if(ack_count == 3)
                break;
        }

        if (elapsed_timer.elapsed() > 10*1000)
        {
            emit wlog(tr("Firmware Write Error"));
            return false;
        }
    }

    QByteArray checkcrc;
    checkcrc.append((uint8_t)((cw >> 8) & 0xFF));
    checkcrc.append((uint8_t)(cw & 0xFF));
    QByteArray r = send(checkcrc, 1);
    if (r[0] != ACK)
    {
        emit wlog("Firmware write ack failed");
        return false;
    }

    emit wlog("Firmware write ok.");
    return true;
}

bool mtkprog::uploadApplication()
{
    if(!da_mem(xFirmware.address, xFirmware.Size, xFirmware.type))
    {
        return false;
    }

    if(!da_write())
    {
        return false;
    }

    if(!da_write_data(xFirmware.data))
    {
        return false;
    }

    return true;
}

bool mtkprog::da_reset()
{
    QByteArray xSend;
    xSend.append((uint8_t)DA_CLEAR_POWERKEY_IN_META_MODE_CMD);
    QByteArray r = send(xSend, 1);  // <-- 5A

    xSend.clear();
    xSend.append((uint8_t)0xC9);
    xSend.append((uint8_t)0x00);
    r = send(xSend, 1);  // ???<-- 5A

    xSend.clear();
    xSend.append((uint8_t)DA_ENABLE_WATCHDOG_CMD);
    xSend.append((uint8_t)0x01);
    xSend.append((uint8_t)0x40);
    xSend.append((uint8_t)0x00);
    xSend.append((uint8_t)0x00);
    xSend.append((uint8_t)0x00);
    xSend.append((uint8_t)0x00);
    r = send(xSend, 1);  // <-- 5A, RESET
    emit wlog("Reset Ok");
    return true;
}

mtkprog::mtk_status_t mtkprog::getStatus(void)
{
    return status;
}

void mtkprog::Start(void)
{
    status = mtk_working;
    /*check firmware*/
    if(xFirmware_Path == "")
    {
        emit wlog(QString("Please select firmware!"));
        die();
        return;
    }

    /*try load firmware*/
    if(!loadfirmware())
    {
        emit wlog(QString("can not load firmware : %1").arg(xFirmware_Path));
        die();
        return;
    }

    xPort = new QSerialPort(xPort_PortName);


    if(!open())
    {
        emit wlog(QString("Can not open Port {%1}").arg(xPort_PortName));
        die();
        return;
    }
    emit wlog(QString("Open Port {%1}").arg(xPort_PortName));

    if(!connect())
    {
        emit wlog(QString("Can not Connect to device!"));
        die();
        return;
    }

    if(!da_start())
    {
        emit wlog(QString("Can not upload bootloader!"));
        die();
        return;
    }

    if(!da_changebaud(BaudRate))
    {
        emit wlog(QString("Can not set baud reate"));
        die();
        return;
    }

    if(!uploadApplication())
    {
        emit wlog(QString("Can not Upload Fremware"));
        die();
        return;
    }

    if(!da_reset())
    {
        emit wlog(QString("reset faild"));
        die();
        return;
    }

    emit wlog("firmware upload ok");
    die(false);
}

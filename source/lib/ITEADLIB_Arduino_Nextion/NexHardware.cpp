/**
 * @file NexHardware.cpp
 *
 * The implementation of base API for using Nextion. 
 *
 * @author  Wu Pengfei (email:<pengfei.wu@itead.cc>)
 * @date    2015/8/11
 * @copyright 
 * Copyright (C) 2014-2015 ITEAD Intelligent Systems Co., Ltd. \n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#include "NexHardware.h"

#define NEX_RET_CMD_FINISHED            (0x01)
#define NEX_RET_EVENT_LAUNCHED          (0x88)
#define NEX_RET_EVENT_UPGRADED          (0x89)
#define NEX_RET_EVENT_TOUCH_HEAD            (0x65)     
#define NEX_RET_EVENT_POSITION_HEAD         (0x67)
#define NEX_RET_EVENT_SLEEP_POSITION_HEAD   (0x68)
#define NEX_RET_CURRENT_PAGE_ID_HEAD        (0x66)
#define NEX_RET_STRING_HEAD                 (0x70)
#define NEX_RET_NUMBER_HEAD                 (0x71)
#define NEX_RET_INVALID_CMD             (0x00)
#define NEX_RET_INVALID_COMPONENT_ID    (0x02)
#define NEX_RET_INVALID_PAGE_ID         (0x03)
#define NEX_RET_INVALID_PICTURE_ID      (0x04)
#define NEX_RET_INVALID_FONT_ID         (0x05)
#define NEX_RET_INVALID_BAUD            (0x11)
#define NEX_RET_INVALID_VARIABLE        (0x1A)
#define NEX_RET_INVALID_OPERATION       (0x1B)

/*
 * Receive uint32_t data. 
 * 
 * @param number - save uint32_t data. 
 * @param timeout - set timeout time. 
 *
 * @retval true - success. 
 * @retval false - failed.
 *
 */
bool recvRetNumber(uint32_t *number, uint32_t timeout)
{
    bool ret = false;
    uint8_t temp[8] = {0};

    if (!number)
    {
        goto __return;
    }
    
    nexSerial.setTimeout(timeout);
    if (sizeof(temp) != nexSerial.readBytes((char *)temp, sizeof(temp)))
    {
        goto __return;
    }

    if (temp[0] == NEX_RET_NUMBER_HEAD
        && temp[5] == 0xFF
        && temp[6] == 0xFF
        && temp[7] == 0xFF
        )
    {
        *number = ((uint32_t)temp[4] << 24) | ((uint32_t)temp[3] << 16) | (temp[2] << 8) | (temp[1]);
        ret = true;
    }

__return:

    if (ret) 
    {
        dbSerialPrint("recvRetNumber :");
        dbSerialPrintln(*number);
    }
    else
    {
        dbSerialPrintln("recvRetNumber err");
    }
    
    return ret;
}


/*
 * Receive string data. 
 * 
 * @param buffer - save string data. 
 * @param len - string buffer length. 
 * @param timeout - set timeout time. 
 *
 * @return the length of string buffer.
 *
 */
uint16_t recvRetString(char *buffer, uint16_t len, uint32_t timeout)
{
    uint16_t ret = 0;
    bool str_start_flag = false;
    uint8_t cnt_0xff = 0;
    String temp = String("");
    uint8_t c = 0;
    long start;

    if (!buffer || len == 0)
    {
        goto __return;
    }
    
    start = millis();
    while (millis() - start <= timeout)
    {
        while (nexSerial.available())
        {
            c = nexSerial.read();
            if (str_start_flag)
            {
                if (0xFF == c)
                {
                    cnt_0xff++;                    
                    if (cnt_0xff >= 3)
                    {
                        break;
                    }
                }
                else
                {
                    temp += (char)c;
                }
            }
            else if (NEX_RET_STRING_HEAD == c)
            {
                str_start_flag = true;
            }
        }
        
        if (cnt_0xff >= 3)
        {
            break;
        }
    }

    ret = temp.length();
    ret = ret > len ? len : ret;
    strncpy(buffer, temp.c_str(), ret);
    
__return:

    dbSerialPrint("recvRetString[");
    dbSerialPrint(temp.length());
    dbSerialPrint(",");
    dbSerialPrint(temp);
    dbSerialPrintln("]");

    return ret;
}

/*
 * Send command to Nextion.
 *
 * @param cmd - the string of command.
 */
void sendCommand(const char* cmd)
{
    while (nexSerial.available())
    {
        nexSerial.read();
    }
    
    nexSerial.print(cmd);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
    nexSerial.write(0xFF);
}


/*
 * Command is executed successfully. 
 *
 * @param timeout - set timeout time.
 *
 * @retval true - success.
 * @retval false - failed. 
 *
 */
bool recvRetCommandFinished(uint32_t timeout)
{    
    bool ret = false;
    uint8_t temp[4] = {0};
    
    nexSerial.setTimeout(timeout);
			
    if (sizeof(temp) != nexSerial.readBytes((char *)temp, sizeof(temp)))
    {
        ret = false;
    }
    if (temp[0] == NEX_RET_CMD_FINISHED
        && temp[1] == 0xFF
        && temp[2] == 0xFF
        && temp[3] == 0xFF
        )
    {
        ret = true;
    }

    if (ret) 
    {
        dbSerialPrintln("recvRetCommandFinished ok");
    }
    else
    {
        dbSerialPrintln("recvRetCommandFinished err");
    }
    
    return ret;
}

uint8_t recvData(uint8_t* data, uint8_t len, uint32_t timeout)
{
    uint8_t blen = 0;

    nexSerial.setTimeout(timeout);
			
    blen = nexSerial.readBytes((char *)data, len);

    char buffer[30];
    sprintf(buffer,"recvData: %d ",blen);
    dbSerialPrintln(buffer);
    
    if (blen >= 1)
    {
        String out;
        for (int i=0; i<blen; i++)
        {
            sprintf(buffer,"%02X ",data[i]);
            out += buffer;
        }
        dbSerialPrintln(out.c_str());
    }
    
    return blen;
}

uint8_t nexSetBaud(uint32_t baudold, String& baudnew)
{
    uint8_t buf[100] = {0};
    uint32_t tout = 500;
    String out("baud=");

    out += baudnew;
    dbSerialPrint("send: ");
    dbSerialPrintln(out.c_str());
    sendCommand(out.c_str());
    if(recvData(buf, 100, tout) == 4 && buf[0] == 1)
        return 1;

    baudnew = "";
    return buf[0];
}

uint32_t nexGetCheckBaud(String& baudret)
{
    uint8_t buf[100] = {0};
    unsigned long b[] = {2400, 4800, 9600, 19200, 38400, 57600, 115200};
    uint16_t len = 0;
    char buffer[30];
    nexSerial.end();
    uint32_t tout = 500;

    for (int i=0; i<7; i++)
    {
        nexSerial.begin(b[i],SERIAL_8N1,nexRXPin,nexTXPin,false);

        sprintf(buffer,"Baud: %ld ",b[i]);
        dbSerialPrintln(buffer);

        dbSerialPrintln("send: \"\"");
        sendCommand("");
        len = recvData(buf, 100, tout);

        dbSerialPrintln("send: bkcmd=3");
        sendCommand("bkcmd=3");
        len = recvData(buf, 100, tout);
        
        dbSerialPrintln("send: get baud");
        sendCommand("get baud");
        len = recvData(buf, 100, tout);

        if (len == 8
            && buf[0] == NEX_RET_NUMBER_HEAD
            && buf[5] == 0xFF
            && buf[6] == 0xFF
            && buf[7] == 0xFF)
        {
            uint32_t n = ((uint32_t)buf[4] << 24) | ((uint32_t)buf[3] << 16) | (buf[2] << 8) | (buf[1]);
            sprintf(buffer,"%d ",n);
            baudret = buffer;
            return n;
        }

        nexSerial.end();
    }
    return 0;
}

uint8_t setNexBaud(String set, String& out)
{
  String sb;

  uint8_t ret = nexSetBaud(nexGetCheckBaud(sb),set);

  if (ret != NEX_RET_CMD_FINISHED)
  {
    char buffer[10];
    sprintf(buffer,"%2X",ret);
    out = "Err: ";
    out += buffer;
    out += " " + sb + "->" + set;
  } 
  else
    out = "Ok " + sb + "->" + set;

  return ret;
}

uint32_t nexInit(uint32_t dbB, uint32_t nexB, String& out)
{
    char buf[8];
    uint8_t buf2[100] = {0};
    uint32_t tout = 500;
    sprintf(buf,"%d",nexB);

    dbSerialBegin(dbB);

    int ret = setNexBaud(buf,out);
    if(ret == 1)
    {
        nexSerial.end();
        nexSerial.begin(nexB,SERIAL_8N1,nexRXPin,nexTXPin,false);

        dbSerialPrintln("send: \"\"");
        sendCommand("");
        uint16_t len = recvData(buf2, 100, tout);
        dbSerialPrintln("send: bkcmd=1");
        sendCommand("bkcmd=1");
        bool ret1 = recvRetCommandFinished();
        dbSerialPrintln("send: page 0");
        sendCommand("page 0");
        bool ret2 = recvRetCommandFinished();
    }
    return ret;
}

bool nexInit()
{
	return nexInit(nexDBBaud);
}

bool nexInit(int baud)
{
    bool ret1 = false;
    bool ret2 = false;
    
    dbSerialBegin(baud);
    nexSerial.begin(9600,SERIAL_8N1,nexRXPin,nexTXPin,false);
    sendCommand("");
    sendCommand("bkcmd=1");
    ret1 = recvRetCommandFinished();
    sendCommand("page 0");
    ret2 = recvRetCommandFinished();
    return ret1 && ret2;
}

void nexLoop(NexTouch *nex_listen_list[])
{
    static uint8_t __buffer[10];
    
    uint16_t i;
    uint8_t c;  
    
    while (nexSerial.available() > 0)
    {   
        delay(10);
        c = nexSerial.read();
        
        if (NEX_RET_EVENT_TOUCH_HEAD == c)
        {
            if (nexSerial.available() >= 6)
            {
                __buffer[0] = c;  
                for (i = 1; i < 7; i++)
                {
                    __buffer[i] = nexSerial.read();
                }
                __buffer[i] = 0x00;
                
                if (0xFF == __buffer[4] && 0xFF == __buffer[5] && 0xFF == __buffer[6])
                {
                    NexTouch::iterate(nex_listen_list, __buffer[1], __buffer[2], (int32_t)__buffer[3]);
                }
                
            }
        }
    }
}


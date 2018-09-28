#include <Arduino.h>
#include "SSD1306Wire.h"
#include <Nextion.h>

uint8_t ledPin = 16;
SSD1306Wire display(0x3c, 5, 4);
uint32_t progTime = 0;
String serout;

String disStr[6];
void disUpdate();

NexButton b0 = NexButton(0, 4, "b0");
NexButton b1 = NexButton(0, 2, "b1");
NexButton b2 = NexButton(0, 5, "b2");
NexButton b3 = NexButton(0, 7, "b2");
NexText t0 = NexText(0,6,"t0");
NexText t1 = NexText(0,3,"t1");
NexText t2 = NexText(0,8,"t2");
NexNumber n0 = NexNumber(0,1,"n0");

char buffer[100] = {0};
NexTouch *nex_listen_list[] = 
{
    &b0,
    &b1,
    &b2,
    &b3,
    NULL
};

void setNexNum(uint32_t nin = -1)
{
    uint32_t number;
    
    n0.getValue(&number);
    
    if (nin <= 0)
      n0.setValue(0);
    else
    {
      number += 1;
      n0.setValue(number);
    }
}

void b0CB(void *ptr)
{
    uint16_t number;
    NexButton *btn = (NexButton *)ptr;
    disStr[3] = "b0CB";

    memset(buffer, 0, sizeof(buffer));
    btn->getText(buffer, sizeof(buffer));
    
    number = atoi(buffer);
    number += 1;

    memset(buffer, 0, sizeof(buffer));
    itoa(number, buffer, 10);
    btn->setText(buffer);

    disStr[4] = buffer;
    disUpdate();
}

void b1CB(void *ptr)
{
    disStr[3] = "b1CB";
    memset(buffer, 0, sizeof(buffer));
    t0.getText(buffer, sizeof(buffer));
    disStr[4] = buffer;
    setNexNum();
    disUpdate();
}

void b2CB(void *ptr)
{
    disStr[3] = "b2CB";
    memset(buffer, 0, sizeof(buffer));
    t1.getText(buffer, sizeof(buffer));
    disStr[4] = buffer;
    setNexNum();
    disUpdate();
}

void b3CB(void *ptr)
{
    disStr[3] = "b3CB";
    memset(buffer, 0, sizeof(buffer));
    t2.getText(buffer, sizeof(buffer));
    disStr[4] = buffer;
    t1.setText(buffer);
    setNexNum();
    disUpdate();
}

void disUpdate()
{
  display.clear();
  display.drawString(0, 0, disStr[0]);
  display.drawString(0, 10, disStr[1]);
  display.drawString(0, 20, disStr[2]);
  display.drawString(0, 30, disStr[3]);
  display.drawString(0, 40, disStr[4]);
  display.drawString(0, 50, disStr[5]);
  display.display();
}

void setup() {
  pinMode(ledPin, OUTPUT);

  display.init();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  disStr[0] = "Start";
  disUpdate();

  for (int i=0; i<=10; i++)
  {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(100);
  }

  disStr[0] = "UART2 RX 21 TX 22";
  disUpdate();

  uint32_t nRet = nexInit(nexDBBaud,nexBaudrate,serout);

  disStr[1] = serout;
  disUpdate();

  b0.attachPop(b0CB, &b0);
  b1.attachPop(b1CB, &b1);
  b2.attachPop(b2CB, &b2);
  b3.attachPop(b3CB, &b3);
  setNexNum(0);
  dbSerialPrintln("setup done"); 
}

void loop() 
{
  nexLoop(nex_listen_list);

  progTime = millis()/1000;

  disStr[2] =  String(progTime);
  disUpdate();
}
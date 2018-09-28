/*

  Demo App for the  Enganced version of the Nextion displays and the ESP32. It runs also on other Arduinos and ESP8266 (if Serial are adapted)

  Includes:
  - 6 Softkeys on the display which beep the beeper if pressed (functionality on display)
  - Keys trigger MQTT messages with number included
  - Time display synchronized with NTP time and then updated by RTC of Nextion display (Read/write of RTC)
  - LED / GPIO6 can be switched on/off by incoming MQTT message
  - Reading GPIOs: Enter Button / GPIO1 creates MQTT message with "E"

   Copyright <2017> <Andreas Spiess>

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.

   Based on the Examples of the libraries
*/

#include <Nextion.h>
#include <NTPtimeESP32.h>
#include <credentials.h>
#include <PubSubClient.h>

HardwareSerial Serial2(2);

#define SENDTOPIC "keyboard/key"
#define COMMANDTOPIC "keyboard/command"
#define SERVICETOPIC "keyboard/service"


// GPIO
NexGpio gpio;

#define   GPIO_RELAY_PORT         6
#define   GPIO_ENTER_PORT         1

#define   GPIO_READ_MODE          0
#define   GPIO_OUTPUT_MODE        2
#define   GPIO_PWM_OUT_MODE       3

#define   CONTROLS_ID             0           //0 for most of the time

//MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  if (payload[0] == '1') {
    gpio.digital_write(GPIO_RELAY_PORT, HIGH);
    Serial.println("ON");
  }
  else {
    gpio.digital_write(GPIO_RELAY_PORT, LOW);
    Serial.println("OFF");
  }
}

IPAddress server(192, 168, 0, 203);

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);


#define KEY1 "1"
#define KEY2 "2"
#define KEY3 "3"
#define KEY4 "4"
#define KEY5 "5"
#define KEY6 "6"
#define ENTERKEY "E"


// NTP
NTPtime NTPch("ch.pool.ntp.org");   // Choose server pool as required

/*
  The structure contains following fields:
  struct strDateTime
  {
  byte hour;
  byte minute;
  byte second;
  int year;
  byte month;
  byte day;
  byte dayofWeek;
  boolean valid;
  };
*/
strDateTime dateTime;

#ifdef CREDENTIALS
char *ssid      = mySSID;               // Set you WiFi SSID
char *password  = myPASSWORD;               // Set you WiFi password
#else
char *ssid      = "";               // Set you WiFi SSID
char *password  = "";               // Set you WiFi password
#endif

NexPage page0 = NexPage(0, 0, "page0");

NexText t0 = NexText(0, 1, "t0");

NexHotspot m1    = NexHotspot(0, 2, "key1");
NexHotspot m2    = NexHotspot(0, 3, "key2");
NexHotspot m3    = NexHotspot(0, 5, "key3");
NexHotspot m4    = NexHotspot(0, 6, "key4");
NexHotspot m5    = NexHotspot(0, 7, "key5");
NexHotspot m6    = NexHotspot(0, 4, "key6");

int number = 50;
char buffer[10] = {0};
unsigned long entry;
bool keyPressed = true, enterKeyPressed=false;

NexRtc  rtc;

uint32_t  rtcTime[7] = {2017, 9, 25, 12, 34, 00};
char time_buf[30] = {0};


/*
  Register object textNumber, buttonPlus, buttonMinus, to the touch event list.
*/
NexTouch *nex_listen_list[] =
{
  &m1,
  &m2,
  &m3,
  &m4,
  &m5,
  &m6,
  NULL
};


void m1PushCallback(void *ptr)
{
  dbSerialPrintln("m1PushCallback");
  dbSerialPrint("ptr = ");
  dbSerialPrintln((uint32_t)ptr);
  keyPressed = true;
  publishMQTT(SENDTOPIC, KEY1);
}

void m2PushCallback(void *ptr)
{
  dbSerialPrintln("m2PopCallback");
  dbSerialPrint("ptr = ");
  dbSerialPrintln((uint32_t)ptr);
  keyPressed = true;
  publishMQTT(SENDTOPIC, KEY2);
}
void m3PushCallback(void *ptr)
{
  dbSerialPrintln("m3PushCallback");
  dbSerialPrint("ptr = ");
  dbSerialPrintln((uint32_t)ptr);
  keyPressed = true;
  publishMQTT(SENDTOPIC, KEY3);
}

void m4PushCallback(void *ptr)
{
  dbSerialPrintln("m4PopCallback");
  dbSerialPrint("ptr = ");
  dbSerialPrintln((uint32_t)ptr);
  keyPressed = true;
  publishMQTT(SENDTOPIC, KEY4);
}
void m5PushCallback(void *ptr)
{
  dbSerialPrintln("m5PushCallback");
  dbSerialPrint("ptr = ");
  dbSerialPrintln((uint32_t)ptr);
  keyPressed = true;
  publishMQTT(SENDTOPIC, KEY5);
}

void m6PushCallback(void *ptr)
{
  dbSerialPrintln("m6PopCallback");
  dbSerialPrint("ptr = ");
  dbSerialPrintln((uint32_t)ptr);
  keyPressed = true;
  publishMQTT(SENDTOPIC, KEY6);
}

//----------------- S E T U P ----------------------------

void setup(void)
{
  nexInit();      // default 115200
  Serial.println("Connecting to Wi - Fi");

  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("WiFi connected");

  while (!dateTime.valid) {
    // getNTPtime: First parameter: Time zone in floating point (for India); second parameter: 1 for European summer time; 2 for US daylight saving time (contributed by viewwer, not tested by me)
    dateTime = NTPch.getNTPtime(1.0, 1);

    // check dateTime.valid before using the returned time
    // Use "setSendInterval" or "setRecvTimeout" if required
    Serial.print(".");
    delay(500);
  }
  NTPch.printDateTime(dateTime);
  rtcTime[0] = dateTime.year;
  rtcTime[1] = dateTime.month;
  rtcTime[2] = dateTime.day;
  rtcTime[3] = dateTime.hour;
  rtcTime[4] = dateTime.minute;

  m1.attachPush(m1PushCallback, &m1);
  m2.attachPush(m2PushCallback, &m2);
  m3.attachPush(m3PushCallback, &m3);
  m4.attachPush(m4PushCallback, &m4);
  m5.attachPush(m5PushCallback, &m5);
  m6.attachPush(m6PushCallback, &m6);

  rtc.write_rtc_time(rtcTime);

  // MQTT

  if (client.connect("Mailbox", "admin", "admin")) {
    client.publish(SERVICETOPIC, "Keyboard live");
    client.subscribe(COMMANDTOPIC);
  }

  // GPIO
  gpio.pin_mode(GPIO_RELAY_PORT, GPIO_OUTPUT_MODE, CONTROLS_ID); // can be defined here or in the display
  gpio.pin_mode(GPIO_ENTER_PORT, GPIO_READ_MODE, CONTROLS_ID);

  dbSerialPrintln("setup done");
}



//------------------- L O O P ----------------------------------------

void loop(void)
{
  if (millis() - entry > 60000 || keyPressed) {
    keyPressed = false;
    entry = millis();
    rtc.read_rtc_time(time_buf, 30);
    Serial.println(time_buf);
    t0.setText(time_buf);
  }

/*  
This part works, but it polls too often and blocks all otther functions of th display

  if (gpio.digital_read(GPIO_ENTER_PORT) == 0) {
    keyPressed = true;
    publishMQTT(SENDTOPIC, ENTERKEY);
  }
  */

  nexLoop(nex_listen_list);
  client.loop();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Mailbox")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(SERVICETOPIC, "I am live again");
      // ... and resubscribe
      //  client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publishMQTT(String topic, String message) {
  if (!client.connected()) {
    reconnect();
  }
  client.publish(topic.c_str(), message.c_str());
}


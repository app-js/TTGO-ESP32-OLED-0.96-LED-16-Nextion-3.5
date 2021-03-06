Playground for

## TTGO WiFi & Bluetooth Battery ESP32 0.96 inch OLED development tool  
## and Nextion Enhanced NX4832K035_11 - Generic 3.5'' HMI Touch Display

#### TTGO Features:

* When you do some ESP32 projects you must hate to add a power bank to provide the power   
* This tiny board could solve such problems  
* A LG 3000mAH 18650 battery could make ESP32 run 17 hours or more  
* 18650 charging system integrated.  
* Indicate LED inside (Green means full & Red means charging)  
* Charging and working could be at the same time.  
* 1 Switch could control the power  
* 1 extra LED could be programmed(Connected with GPIO16[D0])  
* 0.5A charging current   
* 1A output   
* Over charge protection   
* Over discharge protection   
* Full ESP32 pins break out 

![TTGO pic](img/ttgo_p0.jpg)
![pinout](img/ttgo_pinout.jpg)

#### Nextion Features:

* Nextion Enhanced NX4832K035 is a 3.5'' HMI TFT display,  
* with 32MB Flash data storage space,  
* 1024 byte EEPROM,  
* 8192 byte RAM,  
* GPIO support, to control external devices.  

![Nextion pic0](img/nex_p0.jpg)
![Nextion pic1](img/nex_p1.jpg)

#### Connecting TTGO ESP32 and Nextion:

TTGO | Nextion
------------ | -------------
5V | 5V
GND | GND
21 | TX
22 | RX

#### Install and build:

* You must use / install platformio.
* Copy ttgo-oled1306-led16-battery.json and pins_arduino.h to their directories or change [env:ttgo-oled1306-led16-battery] in platformio.ini to your boardname. See "Board and Pin Definitions for platformio"
* You need the nextion library from this package. The most important changes are in nexconfig.h, nexhardware.h and nexhardware.cpp.

![In action](img/inaction.jpg)

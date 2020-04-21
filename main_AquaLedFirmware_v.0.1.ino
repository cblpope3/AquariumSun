#include <Wire.h>
#include <TimeLib.h>
#include "PWM.h"
#include "wifi_classes.h"

uint8_t ledArray[32];
uint16_t brightArray[32]={0};
uint32_t aaa = 0xFFFFFFFF;
LedController leds;

WiFi_operations wifiSTA;
time_t tim;

void printTimeSerial(void);

void setup() {
  //  //thailand sun calendar:
  //  //night: 19:00 - 03:30  - total 510 minutes
  //  //sunrise: 03:30 - 05:00
  //  //day: 05:00 - 17:40
  //  //sunset: 17:40 - 19:00

  //new settings
  // night: 02:00
  // sunrise: 12:00
  // day: 15:00
  // sunset: 23:00
  uint16_t dayTimesArray[8]={2,0,12,0,15,0,23,0};

  Serial.begin(9600);
  Serial.println("=============16 channel PWM test!=============");

  wifiSTA.connectToWifi("","");

  leds.setNewDayTimes(dayTimesArray);  
  leds.initialize();
  leds.controlMode = AUTO;

  setTime(wifiSTA.getTime());
}

void loop() {
  int32_t dif;

  
  if(!(minute()%15)){
    tim = wifiSTA.getTime();
    dif = tim - now();
    if (abs(dif) < 1200) {
      setTime(tim);
      Serial.print("Got new time from NTP server. Time is ");
    } else Serial.print("Got incorrect time! Internal time is ");
  }
  
  printTimeSerial();
  
  leds.lightHandle(now());
  
  for (int i=0; i<30; i++){
    yield();
    delay(1000);
  }
  
}

void printTimeSerial(void){
  Serial.print("Time is: ");
  Serial.print(hour());
  Serial.print(":");
  if(minute()<10)Serial.print("0");
  Serial.print(minute());
  Serial.print(":");
  if(second()<10)Serial.print("0");
  Serial.println(second());
}

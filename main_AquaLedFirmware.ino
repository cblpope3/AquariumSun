#include <Wire.h>
#include <TimeLib.h>
#include "PWM.h"
#include "wifi_classes.h"
#include <EEPROM.h>

//#define TEST_MODE
#define GENERAL_MODE

LedController leds;
WiFi_operations wifiSTA;

time_t tim;

void printTimeSerial(void);
void printEEPROMContent(void);

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
  uint16_t frequency = 150, controlMode = AUTO;

  Serial.begin(9600);
  Serial.println("\n=============16 channel PWM test!=============");

  EEPROM.begin(512);

    #ifdef EEPROM_DEBUG
    printEEPROMContent();
    #endif

  wifiSTA.connectToWifi("","");

  leds.initialize();

  #ifdef GENERAL_MODE
  setTime(wifiSTA.getTime());
  #endif
  
  #ifdef TEST_MODE
  setTime(2,12,0,11,4,2020);
  tim=now();
  #endif
}

void loop() {
  int32_t dif;

  #ifdef GENERAL_MODE
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
  #endif
  
  #ifdef TEST_MODE
  leds.lightHandle(tim);
  tim+=50;

  delay(20);
  yield();
  #endif
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

void printEEPROMContent(void){
  for (int i=0; i<EE_MAX_ADDRESS+1; i++){
    Serial.print("Content of EEPROM register " + String(i) + " is: 0x");
    Serial.println(EEPROM.read(i), HEX);
  }
  return;
}

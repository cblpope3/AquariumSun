#include <Ticker.h>
#include <Wire.h>
#include <TimeLib.h>
#include "PWM.h"
#include "wifi_classes.h"
#include "htmlPage.h"
#include <EEPROM.h>

//#define TEST_MODE
#define GENERAL_MODE
#define WIFI_CONTROL_EN
#define WEB_SERVER_DEBUG
#define TICKER_DEBUG

LedController leds;
WiFi_operations wifiSTA;
ESP8266WebServer server(80);
Ticker timeSync, lightHandle;

void printTimeSerial(void);
void printEEPROMContent(void);
void syncTime(void);
void lightRefresh(void);
void STAWebServerInit(void);
void STAHandleRoot(void);
void handleTiming(void);
void handleBright(void);
void handleFrequency(void);
void handleControl(void);

extern String makeMainHtmlPage(int value);

void setup() {
  //new settings
  // night: 02:00
  // sunrise: 12:00
  // day: 15:00
  // sunset: 23:00
  
  //Uncomment to test leds
  //uint8_t dayTimesArray[8]={2,0,12,0,15,0,23,0};
  //uint8_t newBrightnessArray[5]={0xFF,0xFF,0xFF,0xFF,0x3F};
  //uint16_t frequency = 150, controlMode = AUTO;

  Serial.begin(9600);
  Serial.println("\n=============16 channel PWM test!=============");

  EEPROM.begin(512);

    //Uncomment to test leds
    //leds.setNewDayTimes(dayTimesArray);
    //leds.setNewBrightness(newBrightnessArray);
    //leds.setOtherParameter(PWM_FREQUENCY, frequency);
    //leds.setOtherParameter(CONTROL_MODE, controlMode);

    #ifdef EEPROM_DEBUG
    printEEPROMContent();
    #endif

  wifiSTA.connectToWifi("","");

  leds.initialize();

  #ifdef GENERAL_MODE
  setTime(wifiSTA.getTime());
  #endif

  #ifdef WIFI_CONTROL_EN
  STAWebServerInit();
  #endif

  timeSync.attach(1200, syncTime);
  lightRefresh();
  lightHandle.attach(53, lightRefresh);
}

void loop() {
  #ifdef WIFI_CONTROL_EN
  server.handleClient();
  MDNS.update();
  #endif
  
  yield();
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
  return;
}

void printEEPROMContent(void){
  for (int i=0; i<EE_MAX_ADDRESS+1; i++){
    Serial.print("Content of EEPROM register " + String(i) + " is: 0x");
    Serial.println(EEPROM.read(i), HEX);
  }
  return;
}

void syncTime(void){
  #ifdef TICKER_DEBUG
  Serial.println("TICKER. time sync function entered!");
  #endif
  #ifdef GENERAL_MODE
  time_t refreshedTime, dif;
  
  refreshedTime = wifiSTA.getTime();
  dif = refreshedTime - now();
  if (abs(dif) < 1200) {
    setTime(refreshedTime);
    Serial.print("Got new time from NTP server. Time is ");
  } else Serial.print("Got incorrect time! Internal time is ");

  printTimeSerial();
  #endif
  return;
}

void lightRefresh(void){
  #ifdef TICKER_DEBUG
  Serial.println("TICKER. light handle function entered!");
  #endif
  static time_t testTime=0;
  #ifdef GENERAL_MODE
  leds.lightHandle(now());
  #endif
  #ifdef TEST_MODE
  testTime+=60;
  leds.lightHandle(testTime);
  delay(50);
  #endif
}

void STAWebServerInit(void){
  if (MDNS.begin("aquaLed")) {
    #ifdef WEB_SERVER_DEBUG
    Serial.println("MDNS responder started");
    #endif
  }
  server.on("/", STAHandleRoot);
  server.on("/newtiming/", handleTiming);
  server.on("/newbright/", handleBright);
  server.on("/newfreq/", handleFrequency);
  server.on("/newcmode/", handleControl);
  server.begin();
  return;
}

void STAHandleRoot(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("  wifi client requested root web-page!");
  #endif
  server.send(200, "text/html", makeMainHtmlPage(1));
}

void handleTiming(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("WEBSERVER. wifi client requested page /newtiming/");
  #endif
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    #ifdef WEB_SERVER_DEBUG
    Serial.println("  !!! incorrect request method!");
    #endif
  } else {
    String ddurH, ddurM, dendH, dendM, phchH, phchM;
    //String message = "POST form was:\n";
    
    for (uint8_t i = 0; i < server.args(); i++) {
      //message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
      if (server.argName(i) == "daydur"){
        ddurH = server.arg(i);
        ddurH.remove(ddurH.indexOf(":"));
        ddurM = server.arg(i);
        ddurM.remove(0,ddurM.indexOf(":")+1);
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed day duration: " + ddurH + ":" + ddurM);
        #endif
      }
      else if (server.argName(i) == "dayend") {
        dendH = server.arg(i);
        dendH.remove(dendH.indexOf(":"));
        dendM = server.arg(i);
        dendM.remove(0,dendM.indexOf(":")+1);
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed day end time: " + dendH + ":" + dendM);
        #endif
      }
      else if (server.argName(i) == "phchange") {
        phchH = server.arg(i);
        phchH.remove(phchH.indexOf(":"));
        phchM = server.arg(i);
        phchM.remove(0,phchM.indexOf(":")+1);
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed phase change time: " + phchH + ":" + phchM);
        #endif
      }
    }
    uint16_t dayDurationMinutes = ddurH.toInt()*60 + ddurM.toInt();
    uint16_t dayEndMinutes = dendH.toInt()*60 + dendM.toInt();
    uint16_t phaseChangeMinutes = phchH.toInt()*60 + phchM.toInt();
    #ifdef WEB_SERVER_DEBUG
        Serial.println("  recalculated to minutes day duration: " + String(dayDurationMinutes));
        Serial.println("  recalculated to minutes day end: " + String(dayEndMinutes));
        Serial.println("  recalculated to minutes phase change: " + String(phaseChangeMinutes));
    #endif
    uint16_t sunsetMinutes = dayEndMinutes;
    uint16_t nightMinutes = sunsetMinutes + phaseChangeMinutes;
    if(nightMinutes > 1439) nightMinutes-=1440;
    uint16_t dayMinutes;
    if (sunsetMinutes>dayDurationMinutes) dayMinutes = sunsetMinutes-dayDurationMinutes;
    else dayMinutes = 1440 - sunsetMinutes + dayDurationMinutes;
    uint16_t sunriseMinutes;
    if (dayMinutes>phaseChangeMinutes) sunriseMinutes = dayMinutes-phaseChangeMinutes;
    else sunriseMinutes = 1440 - dayMinutes + phaseChangeMinutes;
    uint16_t timesArray[4]={nightMinutes, sunriseMinutes, dayMinutes, sunsetMinutes};
    #ifdef WEB_SERVER_DEBUG
    Serial.println("  sending array to setNewDayTimes function:");
    for (int i=0; i<4; i++) Serial.println("    period " + String(i) + " time is: " + String(timesArray[i]));
    #endif
    leds.setNewDayTimes(timesArray);
    lightRefresh();
    String message = "New day routine applied successfully!\nDay lasts "+ddurH+" hours "+ddurM+" minutes\nDay ends at "+dendH+":"+dendM+"\nPhase change takes "+phchH+":"+phchM;
    server.send(200, "text/plain", message);
  }
  return;
}

void handleBright(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("WEBSERVER. wifi client requested page /newbright/");
  #endif
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    #ifdef WEB_SERVER_DEBUG
    Serial.println("  !!! incorrect request method!");
    #endif
  } else {
    lightRefresh();
    String message = " NOT IMPLEMENTED";
    server.send(200, "text/plain", message);
  }
  return;
}

void handleFrequency(void){
  
}

void handleControl(void){
  
}

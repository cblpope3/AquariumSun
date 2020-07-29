#include <Ticker.h>
#include <Wire.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <FS.h>
#include "01-wifi_classes.h"
#include "02-lightController.h"
#include "htmlPage.h"
#include "EEPROM_memory.h"

#define AIR_PIN D12
#define CO2_PIN D11
#define airOn() digitalWrite(AIR_PIN, HIGH)
#define airOff() digitalWrite(AIR_PIN, LOW)
#define co2On() digitalWrite(CO2_PIN, LOW)
#define co2Off() digitalWrite(CO2_PIN, HIGH)

//#define TEST_MODE
#define GENERAL_MODE
#define WIFI_CONTROL_EN
#define WEB_SERVER_DEBUG
#define TICKER_DEBUG

PCA9685Driver ledDriver;
LightController light;
WiFi_operations wifiSTA;
ESP8266WebServer server(80);
Ticker tickerTimeSync, tickerLightHandle;

void printTimeSerial(void);
void printEEPROMContent(void);
void syncTime(void);
void lightRefresh(void);
void STAWebServerInit(void);
void STAHandleRoot(void);
void handleSafeMode(void);
void sendTimeString(void);
void sendSettings(void);
void sendDebug(void);
void handleTiming(void);
void handleBright(void);
void handleFrequency(void);
void handleControl(void);
void initCompressor(void);
void handleCompressor (uint8_t curPhase);

void setup() {

  Serial.begin(9600);
  Serial.println("\n=============16 channel PWM test!=============");

  initCompressor();

  ledDriver.begin();
  light.initialize();

  #ifdef EEPROM_DEBUG
  printEEPROMContent();
  #endif

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  wifiSTA.connectToWifi("","");  
  STAWebServerInit();
  
  #ifdef GENERAL_MODE
  //setTime(wifiSTA.getTime());
  #endif
 
  #ifdef WIFI_CONTROL_EN

  #endif

  syncTime();
  delay(50);
  light.lightHandle(now());

  tickerLightHandle.attach(53, lightRefresh);
}

void loop() {
  static uint8_t prevTimeSync=0;
  #ifdef WIFI_CONTROL_EN
  server.handleClient();
  MDNS.update();
  #endif
  if((minute()%3)==0){
    if (minute()!=prevTimeSync){
      prevTimeSync = minute();
      syncTime();  
    }
  }
  handleCompressor(light.curPhase);
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
  Serial.println("EEPROM:");
  EEPROM.begin(EE_MAX_ADDRESS);
  for (int i=0; i<EE_MAX_ADDRESS+1; i++){
    Serial.print("  Content of EEPROM register " + String(i) + " is: 0x");
    Serial.println(EEPROM.read(i), HEX);
  }
  EEPROM.end();
  return;
}

void syncTime(void){
  Serial.println("TIME SYNC.");
  
  time_t receivedTime1, receivedTime2, diff=1000;
  for (int i=0; i<5; i++) {
    receivedTime1 = wifiSTA.getTime();
    if ((hour(receivedTime1)==0)&&(minute(receivedTime1)==0)&&(second(receivedTime1)==0)){
      Serial.println("  !!!GOT ZERO TIME!");
      delay(3000);
      continue;
    }
    delay(10000);
    receivedTime2 = wifiSTA.getTime();
    if ((hour(receivedTime2)==0)&&(minute(receivedTime2)==0)&&(second(receivedTime2)==0)){
      Serial.println("  !!!GOT ZERO TIME!");
      delay(3000);
      continue;
    }
    Serial.print("  Got time:\n  "+String(hour(receivedTime1))+":"+String(minute(receivedTime1))+":"+String(second(receivedTime1)));
    Serial.println("\n  "+String(hour(receivedTime2))+":"+String(minute(receivedTime2))+":"+String(second(receivedTime2)));
    diff = receivedTime2 - receivedTime1;
    if (diff < 50){
      //got correct time!
      setTime(receivedTime2);
      printTimeSerial();
      return; 
    }
  }
  Serial.println("  !!! can't sync time!");
  return;
}

void lightRefresh(void){
  #ifdef TICKER_DEBUG
  Serial.println("TICKER. light handle function entered!");
  #endif
  light.lightHandle(now());
}

void STAWebServerInit(void){
  if (MDNS.begin("aqualed")) {
    #ifdef WEB_SERVER_DEBUG
    Serial.println("MDNS responder started");
    #endif
  }
  
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.on("/safemode/", handleSafeMode);
  server.on("/newtiming/", handleTiming);
  server.on("/newbright/", handleBright);
  server.on("/newfreq/", handleFrequency);
  server.on("/newcmode/", handleControl);
  server.on("/gettime/", sendTimeString);
  server.on("/getsettings/", sendSettings);
  server.on("/getdebug/", sendDebug);
  server.begin();

  MDNS.addService("http", "tcp", 80);
  return;
}

void STAHandleRoot(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("  wifi client requested root web-page!");
  #endif
  server.send(200, "text/html", makeMainHtmlPage());
}

void handleSafeMode(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("WEB SERVER\n  client requested safe mode web page.");
  #endif
  server.send(200, "text/html", makeMainHtmlPage());
}

void sendTimeString(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("WEB SERVER\n  client requested current time");
  #endif
  String currentTime="";
  if (hour()<10) currentTime += "0";
  currentTime += String(hour())+":";
  if (minute()<10) currentTime += "0";
  currentTime += String(minute()) + ":";
  if (second()<10) currentTime += "0";
  currentTime += String(second());
  server.send(200, "text/plain", currentTime);
}

void sendSettings(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("WEB SERVER\n  client requested settings");
  #endif
  String currentSettings = light.getParam(DAY_PH_DUR) + ";" + light.getParam(DAY_PH_END) + ";" + light.getParam(PH_CHANGE_DUR) + ";" + \
    light.getParam(RMAX) + ";" + light.getParam(GMAX) + ";" + light.getParam(BMAX) + ";" + light.getParam(WMAX) + ";" + light.getParam(NMAX) + ";" + \
    light.getParam(FREQ) + ";" + \
    light.getParam(CON_MODE) + ";" + \
    "0.4";

  server.send(200, "text/plain", currentSettings);
}

void sendDebug(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("WEB SERVER\n  client requested debug information");
  #endif
  String phasesOrderString[4], phaseBegin[4];
  for (int i=0;i<4;i++) {
    switch (light.phasesOrder[i]){
      case NIGHT:
        phasesOrderString[i] = "Night";
        break;
      case SUNRISE:
        phasesOrderString[i] = "Sunrise";
        break;
      case DAY:
        phasesOrderString[i] = "Day";
        break;
      case SUNSET:
        phasesOrderString[i] = "Sunset";
        break;
      default:
        phasesOrderString[i] = "NAN";
    }
  }
  for (int i=0;i<4;i++){
    if (light.phaseBeginMinutes[i]/60 < 10) phaseBegin[i]="0";
    phaseBegin[i] += light.phaseBeginMinutes[i]/60;
    phaseBegin[i] += ":";
    if (light.phaseBeginMinutes[i]%60 < 10) phaseBegin[i]+="0";
    phaseBegin[i] += light.phaseBeginMinutes[i]%60;
  }
  
  String debugInfo = phaseBegin[0] + ";" + phaseBegin[1] + ";" + phaseBegin[2] + ";" + phaseBegin[3] + ";" + \
    String(light.phaseDurationMinutes[0]) + ";" + String(light.phaseDurationMinutes[1]) + ";" + String(light.phaseDurationMinutes[2]) + ";" + String(light.phaseDurationMinutes[3]) + ";" + \
    phasesOrderString[0] + ";" + phasesOrderString[1] + ";" + phasesOrderString[2] + ";" + phasesOrderString[3] + ";" + \
    String(light.curPhase) + ";" + String(light.curPhaseFr) + ";" + \
    ESP.getResetReason() + ";" + String(ESP.getFreeHeap()) + ";" + String(ESP.getMaxFreeBlockSize());

  server.send(200, "text/plain", debugInfo);
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
    
    for (uint8_t i = 0; i < server.args(); i++) {
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
    else dayMinutes = 1440 + sunsetMinutes - dayDurationMinutes;
    uint16_t sunriseMinutes;
    if (dayMinutes>phaseChangeMinutes) sunriseMinutes = dayMinutes-phaseChangeMinutes;
    else sunriseMinutes = 1440 + dayMinutes - phaseChangeMinutes;
    #ifdef WEB_SERVER_DEBUG
    Serial.println("  CALCULATED INPUT DATA:");
    Serial.println("    night starts in: " + String(nightMinutes) + " minutes");
    Serial.println("    sunrise starts in: " + String(sunriseMinutes) + " minutes");
    Serial.println("    day starts in: " + String(dayMinutes) + " minutes");
    Serial.println("    sunset starts in: " + String(sunsetMinutes) + " minutes");
    #endif
    uint16_t timesArray[4];
    timesArray[NIGHT]=nightMinutes;
    timesArray[SUNRISE]=sunriseMinutes;
    timesArray[DAY]=dayMinutes;
    timesArray[SUNSET]=sunsetMinutes;
    #ifdef WEB_SERVER_DEBUG
    Serial.println("  sending array to setNewDayTimes function:");
    for (int i=0; i<4; i++) Serial.println("    period " + String(i) + " time is: " + String(timesArray[i]));
    #endif
    light.setNewDayRoutine(timesArray);
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
    Serial.println("  !!! incorrect request method!");
  } else {
    uint8_t colorsArray[5]={0};
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "rmax"){
        String received;
        received = server.arg(i);
        colorsArray[0] = received.toInt();
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed red color max: " + String(colorsArray[0]));
        #endif
      } else if (server.argName(i) == "gmax") {
        String received;
        received = server.arg(i);
        colorsArray[1] = received.toInt();
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed green color max: " + String(colorsArray[1]));
        #endif
      } else if (server.argName(i) == "bmax") {
        String received;
        received = server.arg(i);
        colorsArray[2] = received.toInt();
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed blue max: " + String(colorsArray[2]));
        #endif
      } else if (server.argName(i) == "wmax") {
        String received;
        received = server.arg(i);
        colorsArray[3] = received.toInt();
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed white max: " + String(colorsArray[3]));
        #endif
      } else if (server.argName(i) == "nmax") {
        String received;
        received = server.arg(i);
        colorsArray[4] = received.toInt();
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed night max: " + String(colorsArray[4]));
        #endif
      } else {
        //////////////////////////////////////////////
        //place here error message (wrong argument)
        //////////////////////////////////////////////
      }
    }
    
    light.setNewBrightLimit(colorsArray);
    
    lightRefresh();
    String message = "New colors applied successfully!\nR max: "+String(colorsArray[0])+"\nG max: "+String(colorsArray[1])+\
        "\nB max: "+String(colorsArray[2])+"\nW max: "+String(colorsArray[3])+"\nN max: "+String(colorsArray[4]);
    server.send(200, "text/plain", message);
  }
  return;
}

void handleFrequency(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("WEBSERVER. wifi client requested page /newfreq/");
  #endif
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    Serial.println("  !!! incorrect request method!");
  } else {
    uint16_t newFreq=0;
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "pwmfr"){
        String received;
        received = server.arg(i);
        newFreq = received.toInt();
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed PwM frequency: " + String(newFreq));
        #endif
      } else {
        //////////////////////////////////////////////
        //place here error message (wrong argument)
        //////////////////////////////////////////////
      }
    }
    
    light.setNewFreq(float(newFreq));
    
    lightRefresh();
    String message = "New PWM frequency applied successfully!\nNew PWM frequency: "+String(newFreq);
    server.send(200, "text/plain", message);
  }
  return;
}

void handleControl(void){
  #ifdef WEB_SERVER_DEBUG
  Serial.println("WEBSERVER. wifi client requested page /newcmode/");
  #endif
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    Serial.println("  !!! incorrect request method!");
  } else {
    uint8_t newControlMode=0;
    for (uint8_t i = 0; i < server.args(); i++) {
      if (server.argName(i) == "cmode"){
        String received;
        received = server.arg(i);
        newControlMode = received.toInt();
        #ifdef WEB_SERVER_DEBUG
        Serial.println("  parsed control mode: " + String(newControlMode));
        #endif
      } else {
        //////////////////////////////////////////////
        //place here error message (wrong argument)
        //////////////////////////////////////////////
      }
    }

    light.setNewControlMode(newControlMode);
    
    lightRefresh();
    String message = "New control mode applied successfully!\nNew control mode: "+String(newControlMode);
    server.send(200, "text/plain", message);
  }
  return;
}
void initCompressor(void){
  pinMode(AIR_PIN, OUTPUT);
  pinMode(CO2_PIN, OUTPUT);

  airOn();
  co2Off();
}
void handleCompressor (uint8_t curPhase){
  static uint8_t prevPhase = 255;
  if (curPhase == prevPhase) return;
  else {
    prevPhase = curPhase;
    if (curPhase == DAY){
      airOff();
      co2On();
    }
    else if (curPhase == NIGHT){
      airOn();
      co2Off();
    }
    else{
      airOff();
      co2Off();
    }
  }
  return;
}

#ifndef _WIFI_CLASSES
#define _WIFI_CLASSES

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "htmlPage.h"

#define WIFI_DEBUG
//#define NTP_DEBUG
#define STA_DEBUG

class WiFi_operations {
  public:
    bool connectToWifi(String ssid, String pass);
    time_t getTime(void);

  private:
};

#endif
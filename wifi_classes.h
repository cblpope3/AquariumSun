#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#define WIFI_DEBUG
//#define NTP_DEBUG

class WiFi_operations {
  public:
    bool connectToWifi(String ssid, String pass);
    time_t getTime(void);
    //IPAddress STAIp;//(192, 168, 1, 100); // where xx is the desired IP Address - most sets of numbers 
    // likely will match gateway, must be unique IP address
    //IPAddress STAGateway;//(192, 168, 1, 1); // set gateway to match your network
    //IPAddress STASubnet;//(255, 255, 255, 0);
  private:
};

#include "wifi_classes.h"

bool WiFi_operations::connectToWifi(String ssid, String pass){
  uint8_t tryCount = 0;
  const uint8_t connectionTimeout = 30;
  
  // set subnet mask to match your network
  WiFi.mode(WIFI_STA);
  
  if (ssid.length()){
    //if new SSID and Pass available, trying to connect with new params
    #ifdef WIFI_DEBUG
    Serial.print("Connecting to ");
    Serial.println(ssid);
    #endif

    WiFi.begin(ssid, pass);
  } else {
    //trying to connect with old params
    #ifdef WIFI_DEBUG
    Serial.print("Connecting with saved SSID and password");
    Serial.println(ssid);
    #endif

    WiFi.begin();
  }

  //Trying to connect it will display dots
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    
    #ifdef WIFI_DEBUG
    Serial.print(".");
    #endif
    
    tryCount++;
    if (tryCount > connectionTimeout) {
      //connection failed.
      #ifdef WIFI_DEBUG
      Serial.print("Connection failed!\n");
      #endif
      
      return false;
    }
  }
  #ifdef WIFI_DEBUG
  Serial.println("\nWiFi connected\nSwitched to STA mode");
  Serial.print("Server started\nUse this URL : http://");
  Serial.println(WiFi.localIP());
  #endif

  return true;
}

time_t WiFi_operations::getTime(void){
  time_t receivedTime;
  
  unsigned int localPort = 2390;      
  IPAddress timeServerIP;
  const char* ntpServerName = "time.nist.gov";
  const int NTP_PACKET_SIZE = 48;
  byte packetBuffer[NTP_PACKET_SIZE];

  WiFiUDP udp;
  udp.begin(localPort);
  WiFi.hostByName(ntpServerName, timeServerIP);
  #ifdef NTP_DEBUG
  Serial.println("sending NTP packet...");
  #endif
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(timeServerIP, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
  
  yield();
  delay(1000);
  
  int cb = udp.parsePacket();
  if (cb) {
    #ifdef NTP_DEBUG
    Serial.print("packet received, length = ");
    Serial.println(cb);
    #endif
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;

    #ifdef NTP_DEBUG
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    #endif
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    receivedTime = epoch + 10800;
    #ifdef NTP_DEBUG
    // print Unix time:
    Serial.println(epoch);
    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second
    #endif
  } else receivedTime = 0;
  return receivedTime;
}
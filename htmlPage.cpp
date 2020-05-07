#include "htmlPage.h"
#include "02-lightController.h"
extern LightController light;

String makeMainHtmlPage(void){
  String currentTime;
  
  if (hour()<10) currentTime+="0";
  currentTime += String(hour()) + ":";
  if (minute()<10) currentTime+="0";
  currentTime += String(minute()) + ":";
  if (second()<10) currentTime+="0";
  currentTime += String(second());

  String outputPage;
  outputPage = "<!DOCTYPE html><html><body><h1>AQUAled</h1><h4>CONTROL PANEL</h4><h2>TIME SETTINGS</h2><h2>" + \
  currentTime +\
  "</h2><form action='/newtiming/' method='post'><p>Day phase duration</p><input type='text' value='" + \
  light.getParam(DAY_PH_DUR) + \
  "' name='daydur'><p>Day phase end time</p><input type='text' value='" + \
  light.getParam(DAY_PH_END) + \
  "' name='dayend'><p>Phase change duration</p><input type='text' value='" + \
  light.getParam(PH_CHANGE_DUR) + \
  "' name='phchange'><input type='submit' value='Apply'></form><h2>LED SETTINGS</h2><form action='/newbright/' method='post'><p>Red maximum</p><input type='text' value='" + \
  light.getParam(RMAX) + \
  "' name='rmax'><p>Green maximum</p><input type='text' value='" + \
  light.getParam(GMAX) +\
  "' name='gmax'><p>Blue maximum</p><input type='text' value='" +\
  light.getParam(BMAX) +\
  "' name='bmax'><p>White maximum</p><input type='text' value='" +\
  light.getParam(WMAX) +\
  "' name='wmax'><p>Night maximum</p><input type='text' value='" +\
  light.getParam(NMAX) +\
  "' name='nmax'><input type='submit' value='Apply'></form><h2>OTHER SETTINGS</h2><form  action='/newfreq/' method='post'><p>PWM frequency, Hz</p><input type='text' value='" +\
  light.getParam(FREQ) + \
  "' name='pwmfr'><input type='submit' value='Apply'></form><form  action='/newcmode/' method='post'><p>Control mode</p><input type='radio' name='cmode' value='" +\
  "AUTO" + "'" + \
  "checked" + \
  "><label>Auto</label><input type='radio' name='cmode' value='" +\
  "MANUAL" + "'" +\
  "><label>Manual</label><input type='submit' value='Apply'></form><h4>ESP8266 data:</h4><p>Last reset reason:" +\
  ESP.getResetReason() +\
  "<br>Free RAM:" +\
  String(ESP.getFreeHeap()) +\
  "<br>Maximum RAM block:" +\
  String(ESP.getMaxFreeBlockSize()) + \
  "<br></p></body></html>";
  return outputPage;
}

String makeHtmlHeader (void){
  String output = "<!DOCTYPE html><html><meta charset='UTF-8'><meta name='viewport'content='width=device-width,initial-scale=1'><link rel='stylesheet'href='https://www.w3schools.com/w3css/4/w3.css'><link rel='stylesheet'href='https://www.w3schools.com/lib/w3-theme-black.css'><link rel='stylesheet'href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.3.0/css/font-awesome.min.css'><style>p.lab{float:left;}input.w3-input{max-width:100px;text-align:center;margin-left:auto;}form{width:70%;margin:auto;}.w3-button{margin:20px;}button.w3-button{margin:0px;margin-top:20px;}</style><body><header class='w3-container w3-theme w3-padding w3-center'><h1 class='w3-xxxlarge w3-animate-bottom'>AQUAled</h1><h4>CONTROL PANEL</h4></header>";
  return output;
}

String makeHtmlFooter (void){
  String output = "<footer class='w3-container w3-theme-dark w3-padding-16'style='margin-top:20px'><p>Designed by Nickolay Shmakolay</p><p>Powered by <a href='https://www.w3schools.com/w3css/default.asp'target='_blank'>w3.css</a></p><p>Version 0.4</p></footer></body></html>";
  return output;
}
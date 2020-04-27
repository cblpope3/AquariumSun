//#include <Arduino.h>
#include "htmlPage.h"

String makeMainHtmlPage(int value){
  String outputPage="";
  String currentTime;
  
  if (hour()<10) currentTime+="0";
  currentTime += String(hour()) + ":";
  if (minute()<10) currentTime+="0";
  currentTime += String(minute()) + ":";
  if (second()<10) currentTime+="0";
  currentTime += String(second());
  //String ledCondition;
  
  //if(value == 1) ledCondition = String ("<p>Led is Engaged now</p>");  
  //else ledCondition = String ("<p>Led is Disngaged now</p>");
  
  outputPage += "<!DOCTYPE html><html><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'><link rel='stylesheet' href='https://www.w3schools.com/lib/w3-theme-black.css'><link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.3.0/css/font-awesome.min.css'><style> p.lab {float:left; } input.w3-input{ max-width:100px; text-align:center; margin-left:auto; } form{ width:70%; margin:auto; } .w3-button{ margin:20px; }</style><body><header class='w3-container w3-theme w3-padding w3-center' id='myHeader'> <h1 class='w3-xxxlarge w3-animate-bottom'>AQUAled</h1><h4>CONTROL PANEL</h4></header><div class='w3-row-padding w3-center w3-margin-top'><div class='w3-third'> <div class='w3-card w3-container' style='min-height:460px'> <h2>TIME SETTINGS</h2><br> <i class='fa fa-clock-o w3-margin-bottom w3-text-theme' style='font-size:120px'></i> <h2>";
  outputPage += currentTime; //current time
  outputPage += "</h2> <form action='/newtiming/' method='post'> <div> <p class='lab'>Day phase duration</p> <input class='w3-input' type='text' value='";
  outputPage += leds.getParam(DAY_PH_DUR); //day phase duration
  outputPage += "'name='";
  outputPage += "daydur";
  outputPage += "'> </div> <div class='w3-section'> <p class='lab'>Day phase end time</p> <input class='w3-input' type='text' value='";
  outputPage += leds.getParam(DAY_PH_END); //day phase end
  outputPage += "' name=";
  outputPage += "'dayend'";
  outputPage += "> </div> <div class='w3-section'> <p class='lab'>Phase change duration</p> <input class='w3-input' type='text' value='";
  outputPage += leds.getParam(PH_CHANGE_DUR); //phase change duration
  outputPage += "' name=";
  outputPage += "'phchange'";
  outputPage += "> </div> <input class='w3-button w3-theme' type='submit' value='Apply'></form> </div></div><div class='w3-third'> <div class='w3-card w3-container' style='min-height:460px'> <h2>LED SETTINGS</h2><br> <i class='fa fa-lightbulb-o w3-margin-bottom w3-text-theme' style='font-size:120px'></i><br><form action='/newbright/' method='post'> <div> <p class='lab'>Red maximum</p> <input style='' class='w3-input' type='text' value='";
  outputPage += leds.getParam(RMAX); //red maximum
  outputPage += "' name=";
  outputPage += "'rmax'";
  outputPage += "> </div> <div class='w3-section'> <p class='lab'>Green maximum</p> <input class='w3-input' type='text' value='";
  outputPage += leds.getParam(GMAX); //green maximum
  outputPage += "' name=";
  outputPage += "'gmax'";
  outputPage += "> </div> <div class='w3-section'> <p class='lab'>Blue maximum</p> <input class='w3-input' type='text' value='";
  outputPage += leds.getParam(BMAX); //blue maximum
  outputPage += "' name=";
  outputPage += "'bmax'";
  outputPage += "> <div class='w3-section'> <p class='lab'>White maximum</p> <input class='w3-input' type='text' value='";
  outputPage += leds.getParam(WMAX); //white maximum
  outputPage += "' name=";
  outputPage += "'wmax'";
  outputPage += "> </div> <div class='w3-section'> <p class='lab'>Night maximum</p> <input class='w3-input' type='text' value='";
  outputPage += leds.getParam(NMAX); //moonlight maximum
  outputPage += "' name=";
  outputPage += "'nmax'> </div> </div> <input class='w3-button w3-theme' type='submit' value='Apply'></form> </div></div><div class='w3-third'> <div class='w3-card w3-container' style='min-height:460px'> <h2>OTHER SETTINGS</h2><br> <i class='fa fa-gears w3-margin-bottom w3-text-theme' style='font-size:120px'></i><form action='/newfreq/' method='post'> <div> <p class='lab'>PWM frequency, Hz</p> <input style='' class='w3-input' type='text' value='";
  outputPage += leds.getParam(FREQ); //PWM frequency
  outputPage += "' name=";
  outputPage += "'pwmfr'";
  outputPage += "> </div> <input class='w3-button w3-theme' type='submit' value='Apply'></form><form action='/newcmode/' method='post'> <div> <p class='lab'>Control mode</p> <input id='male' class='w3-radio' type='radio' name=";
  outputPage += "'gender'";
  outputPage += " value=";
  outputPage += "'AUTO'";
  outputPage += " checked> <label>Auto</label> <br> <input id='female' class='w3-radio' type='radio' name=";
  outputPage += "'gender'";
  outputPage += " value=";
  outputPage += "'MANUAL'";
  outputPage += "> <label>Manual</label> <br> </div> <input class='w3-button w3-theme' type='submit' value='Apply'></form> </div></div></div><footer class='w3-container w3-theme-dark w3-padding-16' style='margin-top:20px'> <p>Designed by Nickolay Shmakolay</p> <p>Powered by <a href='https://www.w3schools.com/w3css/default.asp' target='_blank'>w3.css</a></p> <p>Version 0.3</p> <div style='position:relative;bottom:55px;' class='w3-tooltip w3-right'> <span class='w3-text w3-theme-light w3-padding'>Go To Top</span> <a class='w3-text-white' href='#myHeader'><span class='w3-xlarge'> <i class='fa fa-chevron-circle-up'></i></span></a> </div> </footer></body></html>";
  return outputPage;
}

/*
String makeSettingsHtmlPage(void){
  String outputPage;
    outputPage = String ("<!DOCTYPE html><html><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>") + 
    "<body style='margin:0px'><header style='color:white; background-color:black; text-align:center; margin:0px; padding:20px'>" +
    "<h1>AQUAled НАСТРОЙКИ WI-FI</h1></header><div style='width:400px; margin:auto; padding:20px'><form action='newSettings'><br><h2>SSID:</h2>" +
    "<input type='text' name='ssid' style='display:inline-block'><Br><br><h2>ПАРОЛЬ:</h2><input type='text' name='password'><Br>" +
    "<p><input type='submit' value='Применить'></p></form></div></body></html>";
  return outputPage;
}*/
#include "03-LedCandle.h"

////////////////////////
//CONSTRUCTOR METHODS
////////////////////////

LedCandle::LedCandle(){
  initialization();
  for (int i=0;i<7;i++) led[i].setAddr(ledIndexArray[i]);
}

////////////////////////
//PUBLIC METHODS
////////////////////////

void LedCandle::nightLight(uint8_t phase){
  #ifdef NIGHT_DEBUG
  Serial.println("NIGHT LIGHT. entered function");
  #endif
  uint16_t brightArray[7];
  uint32_t colorArray[7];
  for (int i=0;i<7;i++) {
    colorArray[i] = 0x000000FF;
    #ifdef NIGHT_DEBUG
    Serial.println("  color of channel "+String(i)+" is "+String(colorArray[i]));
    #endif
  }
  
  float tmpBrightArray[7]={0}, maxBright;
  float phasetmp = float(phase);
  
  if (phase<10) maxBright = float(LedCandle::nightMaxBright) * phasetmp / 10;
  else if(phase>245) maxBright = float(LedCandle::nightMaxBright) * (255 - phasetmp) / 10;
  else maxBright = float(LedCandle::nightMaxBright);
  
  tmpBrightArray[0] = maxBright*(1-sq(phasetmp/42));
  tmpBrightArray[1] = maxBright*(1-sq((phasetmp-42)/42));
  tmpBrightArray[2] = maxBright*(1-sq((phasetmp-84)/42));
  tmpBrightArray[3] = maxBright*(1-sq((phasetmp-126)/42));
  tmpBrightArray[4] = maxBright*(1-sq((phasetmp-168)/42));
  tmpBrightArray[5] = maxBright*(1-sq((phasetmp-210)/42));
  tmpBrightArray[6] = maxBright*(1-sq((phasetmp-252)/42));
  
  for (int i=0; i<7; i++) brightArray[i] = constrain(tmpBrightArray[i], 0, 0xFFF);
  #ifdef NIGHT_DEBUG
  for (int i=0;i<7; i++) Serial.println("  bright of channel " + String(i) + " is " + String(brightArray[i]));
  #endif
  LedCandle::massBrightChange(colorArray, brightArray);

  return;
}

void LedCandle::dayLight(uint8_t phase){
  #ifdef DAY_DEBUG
  Serial.println("DAY LIGHT. entered function");
  #endif
  uint16_t brightArray[7];
  uint32_t colorArray[7];
  float tmpBrightArray[7], maxBright = 0xFFF;

  uint32_t color = (rMaxBright<<24)|(gMaxBright<<16)|(bMaxBright<<8)|(wMaxBright);
  for (int i=0; i<7; i++) {
    colorArray[i] = color;
    #ifdef DAY_DEBUG
    Serial.print("  color of channel "+String(i)+" is ");
    Serial.println(colorArray[i], HEX);
    #endif
  }
  
  for (int i=0; i<6; i++) tmpBrightArray[i] = maxBright;
  float tmpPhase = (float)phase;
  tmpBrightArray[6] = maxBright*(1-sq((tmpPhase-125)/125));

  for (int i=0; i<7; i++) {
    brightArray[i] = constrain(tmpBrightArray[i], 0, 0xFFF);
    #ifdef DAY_DEBUG
    Serial.println("  bright of channel "+String(i)+" is "+String(brightArray[i]));
    #endif
  }
  
  LedCandle::massBrightChange(colorArray, brightArray);
  return;
}

void LedCandle::transitionLight(uint8_t phase, bool isRaising){
  #ifdef TRANSITION_DEBUG
  if (isRaising)Serial.print("SUNRISE ");
  else Serial.print("SUNSET ");
  Serial.println(" LIGHT. function entered. phase "+String(phase));
  #endif
  float tmpPhase;
  if (isRaising) tmpPhase = 255.0 - (float)phase;
  else tmpPhase = (float)phase;

  float red = ((255-tmpPhase)/100)*255;
  float green = ((255-tmpPhase)/255)*255;
  float blue = (sq(255-tmpPhase)/35000)*255;
  float white = (sq(255-tmpPhase)/65025)*255;

  uint8_t r = byte(constrain(red, 0, rMaxBright));
  uint8_t g = byte(constrain(green, 0, gMaxBright));
  uint8_t b = byte(constrain(blue, 0, bMaxBright));
  uint8_t w = byte(constrain(white, 0, wMaxBright));

  #ifdef TRANSITION_DEBUG
  Serial.print("  red - " + String(r));
  Serial.print(" green - " + String(g));
  Serial.print(" blue - " + String(b));
  Serial.println(" white - " + String(w));
  #endif
  
  uint32_t currentColor = (r<<24)|(g<<16)|(b<<8)|w;

  uint32_t colorArray[7];
  for (int i=0; i<7; i++) colorArray[i] = currentColor; 

  float tmpBrightArray[7];
  float maxBright=0xFFF;
  tmpBrightArray[0] = maxBright * ((42-tmpPhase)/42);
  tmpBrightArray[1] = maxBright * ((84-tmpPhase)/42);
  tmpBrightArray[2] = maxBright * ((126-tmpPhase)/42);
  tmpBrightArray[3] = maxBright * ((168-tmpPhase)/42);
  tmpBrightArray[4] = maxBright * ((210-tmpPhase)/42);
  tmpBrightArray[5] = maxBright * ((255-tmpPhase)/42);
  tmpBrightArray[6] = 0;
  if(isRaising){
    for (int i=0; i<3; i++){
      tmpBrightArray[6] = tmpBrightArray[i];
      tmpBrightArray[i] = tmpBrightArray[5-i];
      tmpBrightArray[5-i] = tmpBrightArray[6];
    }
    tmpBrightArray[6] = 0;
  }

  uint16_t brightArray[7];
  for (int i=0; i<7; i++) brightArray[i] = constrain(tmpBrightArray[i], 0, 0xFFF);

  LedCandle::massBrightChange(colorArray, brightArray);
  return;
}

void LedCandle::AllOn(void){
  uint32_t colorArray[7];
  uint16_t brightArray[7];
  uint32_t maxColor = (rMaxBright<<24)|(gMaxBright<<16)|(bMaxBright<<8)|(wMaxBright);
  for (int i=0; i<7; i++) {
    colorArray[i] = maxColor;
    brightArray[i] = 0xFFF;
  }
  
  LedCandle::massBrightChange(colorArray, brightArray);
}

void LedCandle::brightTest(){
  uint16_t brightArray[7]={0};
  uint32_t colorArray[7]={0};
  for (int i=0; i<7; i++) colorArray[i]=0xFFFFFFFF;
  for (int id=0; id<7; id++){
    //id - cluster ID
    
    for (uint16_t br=0; br<=0xFFF; br+=10){
      //br - brightness
      brightArray[id]=br;
      massBrightChange(colorArray, brightArray);
    }
  }
}
void LedCandle::ledIndexTest(){
  uint16_t bright=0xFFF;
  uint32_t color=0xFFFFFF;
  led[0].setNewBright(color, bright);
  delay(100);
  led[1].setNewBright(color, bright);
  delay(100);
  led[2].setNewBright(color, bright);
  delay(100);
  led[3].setNewBright(color, bright);
  delay(100);
  led[4].setNewBright(color, bright);
  delay(100);
  led[5].setNewBright(color, bright);
  delay(100);
  led[6].setNewBright(color, bright);
  delay(100);
}

void LedCandle::allOff(void){
  uint32_t colorArray[7]={0};
  uint16_t brightArray[7]={0};
  massBrightChange(colorArray, brightArray);
}

////////////////////////////////
//PRIVATE METHODS
////////////////////////////////

void LedCandle::massBrightChange(uint32_t *colorArray, uint16_t *brightArray){
  for (int i=0;i<7;i++) led[i].setNewBright(colorArray[i], brightArray[i]);
}

void LedCandle::initialization(void){
  EEPROM.begin(EE_MAX_ADDRESS);

  //Restoring settings from EERPOM
  rMaxBright = EEPROM.read(EERMAXBRIGHT);
  gMaxBright = EEPROM.read(EEGMAXBRIGHT);
  bMaxBright = EEPROM.read(EEBMAXBRIGHT);
  wMaxBright = EEPROM.read(EEWMAXBRIGHT);
  nightMaxBright = EEPROM.read(EENMAXBRIGHT);
  
  EEPROM.end();
}
#include "PWM.h"

void LedController::initialize(void){
  
  pwm1.begin();
  pwm2.begin();

  //Restoring settings from EERPOM
  LedController::rMaxBright = EEPROM.read(EERMAXBRIGHT);//0xFF;
  LedController::gMaxBright = EEPROM.read(EEGMAXBRIGHT);//0xFF;
  LedController::bMaxBright = EEPROM.read(EEBMAXBRIGHT);//0xFF;
  LedController::wMaxBright = EEPROM.read(EEWMAXBRIGHT);//0xFF;
  LedController::nightMaxBright = EEPROM.read(EENMAXBRIGHT);//0x3F;
  for (int i=0;i<4;i++){
    LedController::phasesOrder[i] = EEPROM.read(EEPHORDER + i);
    
    LedController::phaseBeginMinutes[i] = EEPROM.read(EEPHBEGIN + 2*i)|(EEPROM.read(EEPHBEGIN + 2*i + 1)<<8);
    LedController::phaseDurationMinutes[i] = EEPROM.read(EEPHDURATION + 2*i)|(EEPROM.read(EEPHDURATION + 2*i+1)<<8);
  }
  LedController::controlMode = EEPROM.read(EECONTROLMODE);
  LedController::pwmFrequency = EEPROM.read(EEPWMFREQ)|(EEPROM.read(EEPWMFREQ+1)<<8);

  #ifdef EEPROM_DEBUG
    Serial.println("Restored data from EEPROM:");
    Serial.print(" phases order: ");
    for (int i=0; i<4; i++) {
      Serial.print(phasesOrder[i]);
      Serial.print(", ");
    }
    Serial.print("\n phases begin: ");
    for (int i=0; i<4; i++) {
      Serial.print(phaseBeginMinutes[i]);
      Serial.print(", ");
    }
    Serial.print("\n phases duration: ");
    for (int i=0; i<4; i++) {
      Serial.print(phaseDurationMinutes[i]);
      Serial.print(", ");
    }
    Serial.println("\nControl mode is: "+String(controlMode));
    Serial.println("Frequency of PWM generator is: "+String(pwmFrequency));
  #endif
  
  pwm1.setPWMFreq(pwmFrequency);
  pwm2.setPWMFreq(pwmFrequency);
  for (int i=0; i<16; i++) {
    pwm1.setPWM(i, 0, 0);
    pwm2.setPWM(i, 0, 0);
  }

  #ifdef LEDCONTROLLER_DEBUG
  Serial.println("Led controller initialized!");
  #endif
}

void LedController::setNewDayTimes(uint8_t *newTimesArray){
  uint8_t tmpArr[4], tmp=0, latestPhase;

  //Calculation begining of phases
  for (int i=0; i<4; i++) LedController::phaseBeginMinutes[i] = newTimesArray[2*i]*60 + newTimesArray[2*i+1];

  #ifdef LEDCONTROLLER_DEBUG
  for (int i=0; i<4; i++) Serial.println("Calculated phase " + String(i)+" beginig is "+String(phaseBeginMinutes[i]));
  #endif

  //find out the latest phase of whole day
  for (int i=0; i<4; i++){
    if (phaseBeginMinutes[i]>tmp){
      tmp = phaseBeginMinutes[i];
      latestPhase = i;
    }
  }

  #ifdef LEDCONTROLLER_DEBUG
  Serial.println("Calculated latest phase of day is: " + String(latestPhase));
  #endif

  //fill array of phases in correct order
  LedController::phasesOrder[3] = latestPhase;
  for (int i=0; i<3; i++){
    LedController::phasesOrder[i] = latestPhase + i + 1;
    if(LedController::phasesOrder[i] > 3) LedController::phasesOrder[i]-=4;
  }

  #ifdef LEDCONTROLLER_DEBUG
  Serial.println("Calculated phases of day is: " + String(phasesOrder[0]) + String(phasesOrder[1]) + String(phasesOrder[2])+String(phasesOrder[3]));
  #endif

  //Calculation duration of phases
  phaseDurationMinutes[phasesOrder[3]] = phaseBeginMinutes[phasesOrder[0]] + 1440 - phaseBeginMinutes[phasesOrder[3]];
  for (int i=0; i<3; i++) phaseDurationMinutes[phasesOrder[i]] = phaseBeginMinutes[phasesOrder[i+1]] - phaseBeginMinutes[phasesOrder[i]];

  for (int i=0;i<4;i++){
    tmpArr[0] = phaseBeginMinutes[i]&0xFF;
    tmpArr[1] = phaseBeginMinutes[i]>>8;
    tmpArr[2] = phaseDurationMinutes[i]&0xFF;
    tmpArr[3] = phaseDurationMinutes[i]>>8;
    
    EEPROM.write(EEPHORDER + i, phasesOrder[i]);
    
    EEPROM.write(EEPHBEGIN + 2*i, tmpArr[0]);
    EEPROM.write(EEPHBEGIN + 2*i+1, tmpArr[1]);

    EEPROM.write(EEPHDURATION + 2*i, tmpArr[2]);
    EEPROM.write(EEPHDURATION + 2*i+1, tmpArr[3]);
  }
  if (EEPROM.commit()) {
    #ifdef EEPROM_DEBUG
    Serial.println("New day times saved in EEPROM");
    #endif
  } else {
    #ifdef EEPROM_DEBUG
    Serial.println("ERROR! Saving new day times in EEPROM failed");
    #endif
  }

  #ifdef LEDCONTROLLER_DEBUG
  Serial.println("New day routine applied:");
  Serial.print("Night starts at " + String(LedController::phaseBeginMinutes[NIGHT]));
  Serial.println(" minutes and lasts " + String(LedController::phaseDurationMinutes[NIGHT]) + " minutes");
  Serial.print("Sunrise starts at " + String(LedController::phaseBeginMinutes[SUNRISE]));
  Serial.println(" minutes and lasts " + String(LedController::phaseDurationMinutes[SUNRISE]) + " minutes");
  Serial.print("Day starts at " + String(LedController::phaseBeginMinutes[DAY]));
  Serial.println(" minutes and lasts " + String(LedController::phaseDurationMinutes[DAY]) + " minutes");
  Serial.print("Sunset starts at " + String(LedController::phaseBeginMinutes[SUNSET]));
  Serial.println(" minutes and lasts " + String(LedController::phaseDurationMinutes[SUNSET]) + " minutes");

  #endif
}

void LedController::setNewBrightness(uint8_t *newBrightArray){
  rMaxBright = newBrightArray[0];
  gMaxBright = newBrightArray[1];
  bMaxBright = newBrightArray[2];
  wMaxBright = newBrightArray[3];
  nightMaxBright = newBrightArray[4];

  EEPROM.write(EERMAXBRIGHT, LedController::rMaxBright);
  EEPROM.write(EEGMAXBRIGHT, LedController::gMaxBright);
  EEPROM.write(EEBMAXBRIGHT, LedController::bMaxBright);
  EEPROM.write(EEWMAXBRIGHT, LedController::wMaxBright);
  EEPROM.write(EENMAXBRIGHT, LedController::nightMaxBright);

  if (EEPROM.commit()) {
    #ifdef EEPROM_DEBUG
    Serial.println("New brightness saved in EEPROM");
    #endif
  } else {
    #ifdef EEPROM_DEBUG
    Serial.println("ERROR! New brightness EEPROM commit failed");
    #endif
  }
}

void LedController::setOtherParameter(uint8_t paramNum, uint16_t paramValue){
  switch (paramNum) {
    case PWM_FREQUENCY:
      //setting new PWM frequency
      pwmFrequency = float(paramValue);
      EEPROM.write(EEPWMFREQ, paramValue&0xFF);
      EEPROM.write(EEPWMFREQ+1, paramValue>>8);
      break;
    case CONTROL_MODE:
      //setting new control mode
      controlMode = uint8_t(paramValue);
      EEPROM.write(EECONTROLMODE, LedController::controlMode);
      break;
    default:
      break;
  }
  if (EEPROM.commit()) {
    #ifdef EEPROM_DEBUG
    Serial.println("Parameter " + String(paramNum) + " saved in EEPROM");
    #endif
  } else {
    #ifdef EEPROM_DEBUG
    Serial.println("ERROR! " + String(paramNum) + " saving parameter in EEPROM failed");
    #endif
  }
}

void LedController::lightHandle(time_t currentTime){

  #ifdef LEDCONTROLLER_DEBUG
    Serial.print ("Entered lightHandle! Current time is: ");
    Serial.print(hour(currentTime));
    Serial.print(":");
    Serial.println(minute(currentTime));
  #endif
  
  if (controlMode == AUTO) LedController::realisticDayLight(currentTime);
  else if (controlMode == MANUAL) return;
  
  return;
}

void LedController::realisticDayLight(time_t currentTime){
  //this function calculating current phase of day and fragment of current phase
  uint16_t minutes, tmp=0;
  uint16_t phaseFragment;
  uint16_t currentPhase;

  minutes = hour(currentTime)*60 + minute(currentTime);

  #ifdef LEDCONTROLLER_DEBUG
    Serial.print("  entered realisticDaylight function! current minute of day is: ");
    Serial.println(minutes);
  #endif

  //calculating current phase
  if(minutes >= LedController::phaseBeginMinutes[phasesOrder[3]]) currentPhase = phasesOrder[3];
  else if (minutes >= LedController::phaseBeginMinutes[phasesOrder[2]]) currentPhase = phasesOrder[2];
  else if (minutes >= LedController::phaseBeginMinutes[phasesOrder[1]]) currentPhase = phasesOrder[1];
  else if (minutes >= LedController::phaseBeginMinutes[phasesOrder[0]]) currentPhase = phasesOrder[0];
  else currentPhase = phasesOrder[3];

  #ifdef LEDCONTROLLER_DEBUG
    Serial.println("  Current phase is " + String(currentPhase));
  #endif

  //calculating current fragment of current phase
  if (currentPhase == phasesOrder[3]){
    //phase in corner of day
    if(minutes > phaseBeginMinutes[phasesOrder[3]]) phaseFragment = minutes - phaseBeginMinutes[phasesOrder[3]];
    else phaseFragment = minutes + (1440 - phaseBeginMinutes[phasesOrder[3]]);
    phaseFragment = map(phaseFragment,0,phaseDurationMinutes[phasesOrder[3]],0,255);
  }else{
    //regular phase of day
    phaseFragment = minutes - phaseBeginMinutes[currentPhase];
    phaseFragment = map(phaseFragment, 0, phaseDurationMinutes[phasesOrder[currentPhase]], 0, 255);
  }

  #ifdef LEDCONTROLLER_DEBUG
    Serial.println("  Current fragment of phase is " + String(phaseFragment));
  #endif

  //handling correct light function
  switch (currentPhase){
    case NIGHT:
      LedController::nightLight(phaseFragment);
      break;
    case SUNRISE:
      LedController::sunriseLight(phaseFragment);
      break;
    case DAY:
      LedController::dayLight(phaseFragment);
      break;
    case SUNSET:
      LedController::sunsetLight(phaseFragment);
      break;
    default:
      Serial.println("  !!!Incorrect phase of day: " + String(currentPhase));
      break;
  }
  return;
}

void LedController::nightLight(uint8_t phase){
  uint8_t ledArray[7]={W1,W2,W3,W4,W5,W6,W7};
  float tmpBrightArray[7]={0}, phasetmp, maxBright;
  uint16_t brightArray[7]={0};

  phasetmp = float(phase);
  
  if (phase<10) maxBright = float(LedController::nightMaxBright) * phasetmp / 10;
  else if(phase>245) maxBright = float(LedController::nightMaxBright) * (255 - phasetmp) / 10;
  else maxBright = float(LedController::nightMaxBright);
  
  tmpBrightArray[0] = maxBright*(1-sq(phasetmp/42));
  tmpBrightArray[1] = maxBright*(1-sq((phasetmp-42)/42));
  tmpBrightArray[2] = maxBright*(1-sq((phasetmp-84)/42));
  tmpBrightArray[3] = maxBright*(1-sq((phasetmp-126)/42));
  tmpBrightArray[4] = maxBright*(1-sq((phasetmp-168)/42));
  tmpBrightArray[5] = maxBright*(1-sq((phasetmp-210)/42));
  tmpBrightArray[6] = maxBright*(1-sq((phasetmp-252)/42));
  
  for (int i=0; i<7; i++) brightArray[i] = constrain(tmpBrightArray[i], 0, 0xFFF);
  LedController::massLedChange(ledArray, 7, brightArray);

  return;
}
void LedController::sunriseLight(uint8_t phase){
  uint8_t ledArray[7]={1,2,3,4,5,6,7}, r,g,b,w;
  uint16_t brightArray[7];
  uint32_t currentColor;
  float tmpBrightArray[7], tmpPhase, maxBright = 0xFFF, red, green, blue, white;

  #ifdef SUNRISE_DEBUG
  Serial.print("phase is ");
  Serial.println(phase);
  #endif

  tmpPhase = float(phase);
  
  red = (tmpPhase/100)*255;
  green = (tmpPhase/255)*255;
  blue = (sq(tmpPhase)/35000)*255;
  white = (sq(tmpPhase)/65025)*255;

  r = byte(constrain(red, 0, rMaxBright));
  g = byte(constrain(green, 0, gMaxBright));
  b = byte(constrain(blue, 0, bMaxBright));
  w = byte(constrain(white, 0, wMaxBright));

  #ifdef SUNRISE_DEBUG
  Serial.print("    red - " + String(r));
  Serial.print(" green - " + String(g));
  Serial.print(" blue - " + String(b));
  Serial.println(" white - " + String(w));
  #endif
  
  currentColor = (r<<24)|(g<<16)|(b<<8)|w;

  tmpBrightArray[0] = maxBright * (tmpPhase/42);
  tmpBrightArray[1] = maxBright * ((tmpPhase-42)/42);
  tmpBrightArray[2] = maxBright * ((tmpPhase-84)/42);
  tmpBrightArray[3] = maxBright * ((tmpPhase-126)/42);
  tmpBrightArray[4] = maxBright * ((tmpPhase-168)/42);
  tmpBrightArray[5] = maxBright * ((tmpPhase-210)/42);
  tmpBrightArray[6] = 0;
  
  #ifdef SUNRISE_DEBUG
  for (int i=0; i<7; i++) brightArray[i] = constrain(tmpBrightArray[i], 0, 0xFFF);
  Serial.print("Sending color code is 0x");
  Serial.println(currentColor, HEX);
  #endif
  
  LedController::massRGBLedChange(ledArray, 7, brightArray, currentColor);
  return;
}
void LedController::dayLight(uint8_t phase){
  uint8_t ledArray[7]={1,2,3,4,5,6,7};
  uint16_t brightArray[7];
  uint32_t currentColor;
  float tmpBrightArray[7], tmpPhase, maxBright = 0xFFF;

  tmpPhase = phase;
  currentColor = 0xFFFFFFFF;
  
  for (int i=0; i<6; i++) tmpBrightArray[i] = maxBright;
  tmpBrightArray[6] = maxBright*(1-sq((tmpPhase-125)/125));

  for (int i=0; i<7; i++) brightArray[i] = constrain(tmpBrightArray[i], 0, 0xFFF);

  LedController::massRGBLedChange(ledArray, 7, brightArray, currentColor);
  return;
}
void LedController::sunsetLight(uint8_t phase){
  uint8_t ledArray[7]={1,2,3,4,5,6,7}, r, g, b, w;
  uint16_t brightArray[7];
  uint32_t currentColor;
  float tmpBrightArray[7], tmpPhase, maxBright = 0xFFF, red, green, blue, white;

  tmpPhase = phase;

  red = ((255-tmpPhase)/100)*255;
  green = ((255-tmpPhase)/255)*255;
  blue = (sq(255-tmpPhase)/35000)*255;
  white = (sq(255-tmpPhase)/65025)*255;

  r = byte(constrain(red, 0, rMaxBright));
  g = byte(constrain(green, 0, gMaxBright));
  b = byte(constrain(blue, 0, bMaxBright));
  w = byte(constrain(white, 0, wMaxBright));

  #ifdef SUNSET_DEBUG
  Serial.print("    red - " + String(r));
  Serial.print(" green - " + String(g));
  Serial.print(" blue - " + String(b));
  Serial.println(" white - " + String(w));
  #endif
  
  currentColor = (r<<24)|(g<<16)|(b<<8)|w;

  tmpBrightArray[0] = maxBright * ((42-tmpPhase)/42);
  tmpBrightArray[1] = maxBright * ((84-tmpPhase)/42);
  tmpBrightArray[2] = maxBright * ((126-tmpPhase)/42);
  tmpBrightArray[3] = maxBright * ((168-tmpPhase)/42);
  tmpBrightArray[4] = maxBright * ((210-tmpPhase)/42);
  tmpBrightArray[5] = maxBright * ((255-tmpPhase)/42);
  tmpBrightArray[6] = 0;


  for (int i=0; i<7; i++) brightArray[i] = constrain(tmpBrightArray[i], 0, 0xFFF);

  LedController::massRGBLedChange(ledArray, 7, brightArray, currentColor);
  return;
}

void LedController::massRGBLedChange (uint8_t *ledArray, uint8_t arrayCount, uint16_t *brightArray, uint32_t color){
  uint8_t outputArrayCount=4*arrayCount, outputLedArray[outputArrayCount];
  uint8_t R,G,B,W;
  float Rcolor[arrayCount], Gcolor[arrayCount], Bcolor[arrayCount], Wcolor[arrayCount];
  uint16_t newBrightArray[outputArrayCount];
  uint8_t ledIndexArray[7][4] = {
  {R1, G1, B1, W1},
  {R2, G2, B2, W2},
  {R3, G3, B3, W3},
  {R4, G4, B4, W4},
  {R5, G5, B5, W5},
  {R6, G6, B6, W6},
  {R0, R7, Y1, W7}
  };

  #ifdef MASSRGBCHANGE_DEBUG
  Serial.print("  entered massRGBchange, entered color is 0x");
  Serial.println(color,HEX);
  #endif

  R = color>>24;
  G = (color>>16)&0xFF;
  B = (color>>8)&0xFF;
  W = color&0xFF;
  
  #ifdef MASSRGBCHANGE_DEBUG
  Serial.print("  Colors separated: Red - 0x");
  Serial.print(R, HEX);
  Serial.print(", Green - 0x");
  Serial.print(G, HEX);
  Serial.print(", Blue - 0x");
  Serial.print(B, HEX);
  Serial.print(", White - 0x");
  Serial.println(W, HEX);

  Serial.println("--  Colors with brightness:");
  #endif
  
  for (int i=0; i<arrayCount; i++){
    //fill array with led indexes
    Rcolor[i] = float(R)*brightArray[i]/0xFF;
    Gcolor[i] = float(G)*brightArray[i]/0xFF;
    Bcolor[i] = float(B)*brightArray[i]/0xFF;
    Wcolor[i] = float(W)*brightArray[i]/0xFF;
  }

  #ifdef MASSRGBCHANGE_DEBUG
  Serial.println("  rgb[i] calculated");
  #endif
  
  for (int i=0; i<arrayCount; i++){
    newBrightArray[4*i] = constrain(Rcolor[i], 0, 0xFFF);
    newBrightArray[4*i+1] = constrain(Gcolor[i], 0, 0xFFF);
    newBrightArray[4*i+2] = constrain(Bcolor[i], 0, 0xFFF);
    newBrightArray[4*i+3] = constrain(Wcolor[i], 0, 0xFFF); 
    
    #ifdef MASSRGBCHANGE_DEBUG
    Serial.print("    Led N ");
    Serial.print(i);
    Serial.print("    RED - 0x");
    Serial.print(newBrightArray[4*i],HEX);
    Serial.print("    GREEN - 0x");
    Serial.print(newBrightArray[4*i+1],HEX);
    Serial.print("    BLUE - 0x");
    Serial.print(newBrightArray[4*i+2],HEX);
    Serial.print("    WHITE - 0x");
    Serial.println(newBrightArray[4*i+3],HEX);
    #endif

  }

  #ifdef MASSRGBCHANGE_DEBUG
  Serial.println("  brightness calculated");
  #endif
  
  for (int i=0; i<arrayCount; i++){
    for (int j=0; j<4; j++){
      outputLedArray[4*i+j] = ledIndexArray[ledArray[i]-1][j];
    }
  }

  #ifdef MASSRGBCHANGE_DEBUG
  Serial.println("  led index array calculated");
  #endif
  
  LedController::massLedChangeTurn(outputLedArray, outputArrayCount, newBrightArray);
  
  return;
}

void LedController::yellowControl(uint8_t brightness){
  uint8_t arrayCount=1, ledArray[1]={Y1};
  uint16_t newBrightArray[1];
  newBrightArray[0] = map(brightness, 0, 0xFF, 0, 0xFFF);
  LedController::massLedChange(ledArray, arrayCount, newBrightArray);
  return;
}
void LedController::redSattelitesControl(uint8_t brightness){
  uint8_t arrayCount=2, ledArray[2]={R0, R7};
  uint16_t newBrightArray[2];
  for (int i=0;i<2;i++) newBrightArray[i] = map(brightness, 0, 0xFF, 0, 0xFF);
  LedController::massLedChange(ledArray, arrayCount, newBrightArray);
  return;
}

void LedController::massLedChangeTurn(uint8_t *ledArray, uint8_t arrayCount, uint16_t *newBrightArray){
  uint16_t currentBrightArray[32]={0}, brightDifference;
  uint8_t iterationCounter=0, iterationLimit=100, changeSpeed=10, cyclePause=5;
  bool isRaisingBright;
  
  //find out brightness of all leds before function start
  for (int i=0; i<arrayCount; i++) currentBrightArray[i] = getLedBright(ledArray[i]);

  //changing brightness one led by one
  for (int i=0; i<arrayCount; i++){
    isRaisingBright = currentBrightArray[i] < newBrightArray[i];
    iterationCounter = 0;
    if(isRaisingBright){
      //trying to raise brightness
      while (currentBrightArray[i]<newBrightArray[i]){
        iterationCounter++;
        brightDifference = newBrightArray[i]-currentBrightArray[i];
        if (brightDifference>changeSpeed) currentBrightArray[i]+=changeSpeed;
        else currentBrightArray[i]=newBrightArray[i];

        setLedBright(ledArray[i], currentBrightArray[i]);
        
        if (iterationCounter>iterationLimit) break;
        delay(cyclePause);
      }
    }else{
      while (currentBrightArray[i]>newBrightArray[i]){
        //trying to lower brightness
        iterationCounter++;
        brightDifference = currentBrightArray[i] - newBrightArray[i];
        if (brightDifference > changeSpeed) currentBrightArray[i]-=changeSpeed;
        else currentBrightArray[i] = newBrightArray[i];

        setLedBright(ledArray[i], currentBrightArray[i]);
    
        if(iterationCounter>iterationLimit) break;
        delay(cyclePause);
      }
    }
  }
}

void LedController::massLedChange(uint8_t *ledArray, uint8_t arrayCount, uint16_t *newBrightArray){
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!!LIGHT BLINKING PROBLEM!!!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //@brief Changes multiple led
  //@param ledArray[arrayCount] - array of indexes leds to be changed
  //@param arrayCount - number of leds to be changed
  //@param newBrightArray[arrayCount] - array of new brightness of each led
  uint16_t currentBrightArray[32]={0};
  bool completedLeds[32]={false}, isRaisingCurrentLed[32]={false};
  uint8_t delayCycle = 2, iterationCounter=0, changeSpeed=5;
  uint8_t ledsCompleted=0;
  int16_t brightDifference;

  //find out brightness of all leds before function start
  for (int i=0; i<arrayCount; i++) currentBrightArray[i] = getLedBright(ledArray[i]);

  while(1){
    for (int i=0; i<arrayCount; i++){
      //skip iteration if led is already done
      if (completedLeds[i]) continue;
      else {
        brightDifference = newBrightArray[i] - currentBrightArray[i];
        if (brightDifference == 0) {
          ledsCompleted++;
          completedLeds[i] = true;
        } else if (brightDifference<0){
          //need to lower bright
          if (changeSpeed>abs(brightDifference)) {
            currentBrightArray[i] = newBrightArray[i];
            ledsCompleted++;
            completedLeds[i] = true;
          }
          else currentBrightArray[i]-=changeSpeed;
        } else if (brightDifference>0){
          //need to raise bright
          if (changeSpeed>abs(brightDifference)){
            currentBrightArray[i] = newBrightArray[i];
            ledsCompleted++;
            completedLeds[i] = true;
          }
          else currentBrightArray[i]+=changeSpeed;
        }
        setLedBright(ledArray[i], currentBrightArray[i]);
        delay(delayCycle);
      }

    }
    iterationCounter++;
    if(ledsCompleted>=arrayCount) return;
    if(iterationCounter>1000) return;
  }
}
void LedController::setLedBright(uint8_t ledNum, uint16_t newBright){
  if (ledNum>15) {
    ledNum-=16;
    pwm2.setPWM(ledNum, 0, newBright);
  } else pwm1.setPWM(ledNum, 0, newBright);
  return;  
}
uint16_t LedController::getLedBright(uint8_t ledNum){
  if (ledNum<16) return pwm1.getPWM(ledNum);
  else {
    ledNum -= 16;
    return pwm2.getPWM(ledNum);
  }
}

void LedController::AllOn(void){
  uint8_t ledArray[32], arrayCount;
  uint16_t newBrightArray[32];

  for (int i=0; i<32; i++) {
    ledArray[i] = i;
    newBrightArray[i] = 0xFFF;
  }
  arrayCount = 32;
  
  LedController::massLedChange(ledArray, arrayCount, newBrightArray);
}
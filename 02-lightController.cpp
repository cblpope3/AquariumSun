#include "02-lightController.h"

////////////////////////
//CONSTRUCTOR METHODS
////////////////////////

LightController::LightController(void){
  return;
}

////////////////////////
//PUBLIC METHODS
////////////////////////

void LightController::initialize(void){

  EEPROM.begin(EE_MAX_ADDRESS);
  for (int i=0;i<4;i++){
    LightController::phasesOrder[i] = EEPROM.read(EEPHORDER + i);
    LightController::phaseBeginMinutes[i] = EEPROM.read(EEPHBEGIN + 2*i)|(EEPROM.read(EEPHBEGIN + 2*i + 1)<<8);
    LightController::phaseDurationMinutes[i] = EEPROM.read(EEPHDURATION + 2*i)|(EEPROM.read(EEPHDURATION + 2*i+1)<<8);
  }
  LightController::controlMode = EEPROM.read(EECONTROLMODE);
  EEPROM.end();

  lamp.allOff();

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
    Serial.println("Frequency of PWM generator is: "+String(ledDriver.pwmFreq));
  #endif
}

void LightController::setNewDayRoutine(uint16_t *newTimesArray){
  uint8_t latestPhase;

  #ifdef LIGHTCONTROLLER_DEBUG
  Serial.println("SET NEW DAY ROUTINE function entered");
  #endif

  //Calculation begining of phases
  for (int i=0; i<4; i++) phaseBeginMinutes[i] = newTimesArray[i];

  #ifdef LIGHTCONTROLLER_DEBUG
  for (int i=0; i<4; i++) Serial.println("  Calculated phase " + String(i)+" beginig is "+String(phaseBeginMinutes[i]));
  #endif

  //find out the latest phase of whole day
  uint16_t tmp=0;
  for (int i=0; i<4; i++){
    if (phaseBeginMinutes[i]>tmp){
      tmp = phaseBeginMinutes[i];
      latestPhase = i;
    }
  }

  #ifdef LIGHTCONTROLLER_DEBUG
  Serial.println("  Calculated latest phase of day: " + String(latestPhase));
  #endif

  //fill array of phases in correct order
  LightController::phasesOrder[3] = latestPhase;
  for (int i=0; i<3; i++){
    LightController::phasesOrder[i] = latestPhase + i + 1;
    if(LightController::phasesOrder[i] > 3) LightController::phasesOrder[i]-=4;
  }

  #ifdef LIGHTCONTROLLER_DEBUG
  Serial.println("  Calculated phases order: " + String(phasesOrder[0]) + String(phasesOrder[1]) + String(phasesOrder[2])+String(phasesOrder[3]));
  #endif

  //Calculation duration of phases
  phaseDurationMinutes[phasesOrder[3]] = phaseBeginMinutes[phasesOrder[0]] + 1440 - phaseBeginMinutes[phasesOrder[3]];
  for (int i=0; i<3; i++) phaseDurationMinutes[phasesOrder[i]] = phaseBeginMinutes[phasesOrder[i+1]] - phaseBeginMinutes[phasesOrder[i]];

  #ifdef LIGHTCONTROLLER_DEBUG
  Serial.println("  Phases duration:");
  for (int i=0; i<4; i++) Serial.println("    phase " + String(i) + " duration is: " + String(phaseDurationMinutes[i]));
  #endif

  EEPROM.begin(EE_MAX_ADDRESS);
  for (int i=0;i<4;i++){
    uint8_t tmpArr[4];
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
  EEPROM.end();
  
  #ifdef LIGHTCONTROLLER_DEBUG
  Serial.println("New day routine applied:");
  Serial.print("Night starts at " + String(LightController::phaseBeginMinutes[NIGHT]));
  Serial.println(" minutes and lasts " + String(LightController::phaseDurationMinutes[NIGHT]) + " minutes");
  Serial.print("Sunrise starts at " + String(LightController::phaseBeginMinutes[SUNRISE]));
  Serial.println(" minutes and lasts " + String(LightController::phaseDurationMinutes[SUNRISE]) + " minutes");
  Serial.print("Day starts at " + String(LightController::phaseBeginMinutes[DAY]));
  Serial.println(" minutes and lasts " + String(LightController::phaseDurationMinutes[DAY]) + " minutes");
  Serial.print("Sunset starts at " + String(LightController::phaseBeginMinutes[SUNSET]));
  Serial.println(" minutes and lasts " + String(LightController::phaseDurationMinutes[SUNSET]) + " minutes");
  #endif
}

void LightController::setNewBrightLimit(uint8_t *newBrightArray){
  lamp.rMaxBright = newBrightArray[0];
  lamp.gMaxBright = newBrightArray[1];
  lamp.bMaxBright = newBrightArray[2];
  lamp.wMaxBright = newBrightArray[3];
  lamp.nightMaxBright = newBrightArray[4];

  EEPROM.begin(EE_MAX_ADDRESS);

  EEPROM.write(EERMAXBRIGHT, lamp.rMaxBright);
  EEPROM.write(EEGMAXBRIGHT, lamp.gMaxBright);
  EEPROM.write(EEBMAXBRIGHT, lamp.bMaxBright);
  EEPROM.write(EEWMAXBRIGHT, lamp.wMaxBright);
  EEPROM.write(EENMAXBRIGHT, lamp.nightMaxBright);

  if (EEPROM.commit()) {
    #ifdef EEPROM_DEBUG
    Serial.println("New brightness saved in EEPROM");
    #endif
  } else {
    #ifdef EEPROM_DEBUG
    Serial.println("ERROR! New brightness EEPROM commit failed");
    #endif
  }
  EEPROM.end();
}

void LightController::setNewFreq(float freq){
  //setting new PWM frequency
  uint16_t frequency = (uint16_t)freq;

  ledDriver.setPWMFreq(freq);

  EEPROM.begin(EE_MAX_ADDRESS);
  EEPROM.write(EEPWMFREQ, frequency&0xFF);
  EEPROM.write(EEPWMFREQ+1, frequency>>8);

  if (EEPROM.commit()) {
    #ifdef EEPROM_DEBUG
    Serial.println("New PWM frequency saved in EEPROM");
    #endif
  } else {
    #ifdef EEPROM_DEBUG
    Serial.println("ERROR! saving new PWM frequency in EEPROM failed");
    #endif
  }
  EEPROM.end();
}

void LightController::setNewControlMode(uint8_t newMode){
  //setting new control mode
  
  LightController::controlMode = newMode;
  
  EEPROM.begin(EE_MAX_ADDRESS);
  EEPROM.write(EECONTROLMODE, LightController::controlMode);

  if (EEPROM.commit()) {
    #ifdef EEPROM_DEBUG
    Serial.println("New control mode saved in EEPROM");
    #endif
  } else {
    #ifdef EEPROM_DEBUG
    Serial.println("ERROR! saving new control mode in EEPROM failed");
    #endif
  }
  EEPROM.end();
}


void LightController::lightHandle(time_t currentTime){

  #ifdef LIGHTCONTROLLER_DEBUG
    Serial.print ("Entered lightHandle! Current time is: ");
    Serial.print(hour(currentTime));
    Serial.print(":");
    Serial.println(minute(currentTime));
  #endif
  
  if (controlMode == AUTO) LightController::realisticDayLight(currentTime);
  else if (controlMode == MANUAL) return;
  
  return;
}


String LightController::getParam(uint8_t param){
  uint8_t maxBrightArray[5];
  String output="";
  switch (param){
    case DAY_PH_DUR:
      output += String(phaseDurationMinutes[DAY]/60);
      output += ":";
      output += String(phaseDurationMinutes[DAY]%60);
      return output;
    case DAY_PH_END:
      output += String(phaseBeginMinutes[((DAY+1)<4)?DAY+1:DAY-3]/60);
      output += ":";
      output += String(phaseBeginMinutes[((DAY+1)<4)?DAY+1:DAY-3]%60);
      return output;
    case PH_CHANGE_DUR:
      output += String(phaseDurationMinutes[SUNRISE]/60);
      output += ":";
      output += String(phaseDurationMinutes[SUNRISE]%60);
      return output;
    case RMAX:
      output += String(lamp.rMaxBright);
      return output;
    case GMAX:
      output += String(lamp.gMaxBright);
      return output;
    case BMAX:
      output += String(lamp.bMaxBright);
      return output;
    case WMAX:
      output += String(lamp.wMaxBright);
      return output;
    case NMAX:
      output += String(lamp.nightMaxBright);
      return output;
    case FREQ:
      output += String(ledDriver.pwmFreq);
      return output;
    case CON_MODE:
      output += String(controlMode);
      return output;
    default:
      output += "#error";
      return output;
  }  
}

void LightController::realisticDayLight(time_t currentTime){
  //this function calculating current phase of day and fragment of current phase
  uint16_t minutes = hour(currentTime)*60 + minute(currentTime);

  #ifdef LIGHTCONTROLLER_DEBUG
    Serial.println("REALISTIC DAY LIGHT.\n  current minute of day is: " + String(minutes));
  #endif

  //calculating current phase
  uint16_t currentPhase;
  if(minutes >= LightController::phaseBeginMinutes[phasesOrder[3]]) currentPhase = phasesOrder[3];
  else if (minutes >= LightController::phaseBeginMinutes[phasesOrder[2]]) currentPhase = phasesOrder[2];
  else if (minutes >= LightController::phaseBeginMinutes[phasesOrder[1]]) currentPhase = phasesOrder[1];
  else if (minutes >= LightController::phaseBeginMinutes[phasesOrder[0]]) currentPhase = phasesOrder[0];
  else currentPhase = phasesOrder[3];
  curPhase = currentPhase;
  #ifdef LIGHTCONTROLLER_DEBUG
    Serial.println("  Phase is " + String(currentPhase));
  #endif

  //calculating current fragment of current phase
  uint16_t phaseFragment;
  if (currentPhase == phasesOrder[3]){
    //phase in corner of day
    if(minutes >= phaseBeginMinutes[currentPhase]) phaseFragment = minutes - phaseBeginMinutes[currentPhase];
    else phaseFragment = minutes + (1440 - phaseBeginMinutes[currentPhase]);
    phaseFragment = map(phaseFragment,0,phaseDurationMinutes[currentPhase],0,255);
  } else {
    //regular phase of day
    phaseFragment = minutes - phaseBeginMinutes[currentPhase];
    phaseFragment = map(phaseFragment, 0, phaseDurationMinutes[currentPhase], 0, 255);
  }
  curPhaseFr = phaseFragment;
  #ifdef LIGHTCONTROLLER_DEBUG
    Serial.println("  Fragment of phase is " + String(phaseFragment));
  #endif

  //handling correct light function
  switch (currentPhase){
    case NIGHT:
      lamp.nightLight(phaseFragment);
      break;
    case SUNRISE:
      lamp.transitionLight(phaseFragment, true);
      break;
    case DAY:
      lamp.dayLight(phaseFragment);
      break;
    case SUNSET:
      lamp.transitionLight(phaseFragment, false);
      break;
    default:
      Serial.println("  !!!Incorrect phase of day: " + String(currentPhase));
      break;
  }
  return;
}
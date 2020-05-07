#include "04-ledCluster.h"

////////////////////////
//CONSTRUCTOR METHODS
////////////////////////

LedCluster::LedCluster(){
  for (int i=0; i<4; i++) {
    chAddr[i]=i;
    chBright[i]=0;
  }
  ledDriver.begin();
}

LedCluster::LedCluster(uint8_t *addrArray){
  LedCluster::setAddr(addrArray);
  ledDriver.begin();
  setOff();
}

void LedCluster::setAddr(uint8_t *addrArray){
  for (int i=0; i<4; i++) {
    chAddr[i]=addrArray[i];
    chBright[i]=0;
  }
}

void LedCluster::setNewBright(uint32_t colorCode, uint16_t brightness){
  setNewChBright(colorCode, brightness);
  //setting new brightness to each led in cluster
  for (int i=0; i<4; i++) ledDriver.setPin(chAddr[i], chBright[i]);
}

void LedCluster::setOff(void){
  setNewBright(0x00000000, 0x0000);
}

////////////////////////////
//PRIVATE FUNCTIONS
////////////////////////////

void LedCluster::setNewChBright(uint32_t colorCode, uint16_t brightness){
  //parsing colorCode to brightness array
  for (int i=0; i<4; i++) chBright[i] = (colorCode>>((3-i)*8))&0xFF;
  //applying maximum brightness to each channel
  for (int i=0; i<4; i++) chBright[i] = map(chBright[i], 0, 0xFF, 0, brightness);
}

#ifndef _LED_CLUSTER
#define _LED_CLUSTER

#include <Arduino.h>
#include "05-PCA9685.h"

extern PCA9685Driver ledDriver;

class LedCluster {
  public:
    LedCluster();
    LedCluster(uint8_t *addrArray);
    void setAddr(uint8_t *addrArray);
    void setNewBright(uint32_t colorCode, uint16_t brightness);
    void setOff(void);
  private:
    uint8_t chAddr[4];
    uint8_t changeDelta=5;
    uint16_t chBright[4];

    void setNewChBright(uint32_t colorCode, uint16_t brightness);
};

#endif

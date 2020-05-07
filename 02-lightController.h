#ifndef _LIGHTCONTROLLER
#define _LIGHTCONTROLLER

#include <TimeLib.h>
#include <EEPROM.h>
#include <Arduino.h>
#include "EEPROM_memory.h"
#include "03-LedCandle.h"

#define LIGHTCONTROLLER_DEBUG
//#define NIGHT_DEBUG
//#define SUNRISE_DEBUG
//#define DAY_DEBUG
//#define SUNSET_DEBUG
//#define MASSRGBCHANGE_DEBUG

#define AUTO 1
#define MANUAL 2

#define PWM_FREQUENCY 0
#define CONTROL_MODE 1

#define NIGHT 0
#define SUNRISE 1
#define DAY 2
#define SUNSET 3

#define DAY_PH_DUR 0
#define DAY_PH_END 1
#define PH_CHANGE_DUR 2
#define RMAX 3
#define GMAX 4
#define BMAX 5
#define WMAX 6
#define NMAX 7
#define FREQ 8
#define CON_MODE 9

class LightController{
  public:
    LightController(void);

    void initialize(void);
    
    void setNewDayRoutine(uint16_t *newTimesArray);
    void setNewBrightLimit(uint8_t *newMaxBrightArray);
    void setNewFreq(float freq);
    void setNewControlMode(uint8_t newMode);

    void lightHandle(time_t currentTime);

    String getParam(uint8_t param);
    uint8_t curPhase, curPhaseFr;
  private:
    LedCandle lamp;

    uint16_t phaseBeginMinutes[4], phaseDurationMinutes[4];
    uint8_t phasesOrder[4];

    uint8_t controlMode;

    void realisticDayLight(time_t currentTime);
};
#endif
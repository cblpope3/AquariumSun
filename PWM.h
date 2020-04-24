#ifndef _PWMLedClass
#define _PWMLedClass

#include <Adafruit_PWMServoDriver.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include "EEPROM_memory.h"

#define LEDCONTROLLER_DEBUG
//#define NIGHT_DEBUG
//#define SUNRISE_DEBUG
//#define DAY_DEBUG
//#define SUNSET_DEBUG
//#define MASSRGBCHANGE_DEBUG

//################################
#define R0 0
#define R1 3
#define R2 9
#define R3 13
#define R4 18
#define R5 22
#define R6 28
#define R7 31
#define W1 1
#define W2 5
#define W3 11
#define W4 15
#define W5 20
#define W6 25
#define W7 30
#define B1 2
#define B2 6
#define B3 12
#define B4 17
#define B5 21
#define B6 27
#define G1 4
#define G2 10  //not working
#define G3 14
#define G4 19
#define G5 23
#define G6 29
#define Y1 24
//####################################

#define AUTO 1
#define MANUAL 2

#define PWM_FREQUENCY 0
#define CONTROL_MODE 1

#define NIGHT 0
#define SUNRISE 1
#define DAY 2
#define SUNSET 3

class LedController{
  public:
    uint8_t controlMode;
    
    void initialize(void);
    void setNewDayTimes(uint8_t *newTimesArray);
    void lightHandle(time_t currentTime);
    void AllOn(void);
    void setNewBrightness(uint8_t *newBrightArray);
    void setOtherParameter(uint8_t paramNum, uint16_t paramValue);
    //void setControlMode(uint8_t newControlMode);

  private:
    Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x40);
    Adafruit_PWMServoDriver pwm2 = Adafruit_PWMServoDriver(0x41);
    uint16_t phaseBeginMinutes[4], phaseDurationMinutes[4];
    uint8_t phasesOrder[4];
    //minutesNight, minutesSunrise, minutesDay, minutesSunset;
    //    uint16_t nightDuration, sunriseDuration, dayDuration, sunsetDuration;
    uint8_t rMaxBright, gMaxBright, bMaxBright, wMaxBright, nightMaxBright;
    
    float pwmFrequency;// = 100;

    void realisticDayLight(time_t currentTime);
    void nightLight(uint8_t phase);
    void sunriseLight(uint8_t phase);
    void dayLight(uint8_t phase);
    void sunsetLight(uint8_t phase);

    void yellowControl(uint8_t brightness);
    void redSattelitesControl(uint8_t brightness);

    void massRGBLedChange (uint8_t *ledArray, uint8_t arrayCount, uint16_t *brightArray, uint32_t color);
    void massLedChangeTurn(uint8_t *ledArray, uint8_t arrayCount, uint16_t *newBrightArray);
    void massLedChange(uint8_t *ledArray, uint8_t arrayCount, uint16_t *newBrightArray);
    
    void setLedBright(uint8_t ledNum, uint16_t newBright);
    uint16_t getLedBright(uint8_t ledNum);
};

#endif
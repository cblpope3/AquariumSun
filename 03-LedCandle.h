#ifndef _LEDCANDLE
#define _LEDCANDLE

#include <EEPROM.h>
#include <Arduino.h>
#include "EEPROM_memory.h"
#include "04-ledCluster.h"

//#define LEDCONTROLLER_DEBUG
//#define NIGHT_DEBUG
//#define TRANSITION_DEBUG
//#define DAY_DEBUG

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

class LedCandle{
  public:
    LedCandle();

    void nightLight(uint8_t phase);
    void dayLight(uint8_t phase);
    void transitionLight(uint8_t phase, bool isRaising);

    void AllOn(void);

    void brightTest(void);
    void ledIndexTest(void);

    void allOff(void);
    uint8_t rMaxBright, gMaxBright, bMaxBright, wMaxBright, nightMaxBright;
  private:
    LedCluster led[7];

    uint8_t ledIndexArray[7][4] = {
                    {R1, G1, B1, W1},
                    {R2, G2, B2, W2},
                    {R3, G3, B3, W3},
                    {R4, G4, B4, W4},
                    {R5, G5, B5, W5},
                    {R6, G6, B6, W6},
                    {R0, R7, Y1, W7}
                  };
    
    float pwmFrequency;

    void massBrightChange(uint32_t *colorArray, uint16_t *brightArray);    
    void initialization(void);
};

#endif

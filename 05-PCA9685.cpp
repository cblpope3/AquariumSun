#include "05-PCA9685.h"

//uncomment next line to receive debug messages
//#define PCA9685_DEBUG

////////////////////////
//CONSTRUCTOR METHODS
////////////////////////

PCA9685Driver::PCA9685Driver()
    : _i2caddr1(PCA9685_I2C_ADDRESS), _i2caddr2(PCA9685_I2C_ADDRESS2), _i2c(&Wire) {}

PCA9685Driver::PCA9685Driver(const uint8_t addr1, const uint8_t addr2)
    : _i2caddr1(addr1), _i2caddr2(addr2), _i2c(&Wire) {}


void PCA9685Driver::begin(void) {
  _i2c->begin();
  reset();
  setPWMFreq(200);
  
  setOscillatorFrequency(FREQUENCY_OSCILLATOR);
  setOCHBit();
}

void PCA9685Driver::setPWMFreq(float freq) {
  #ifdef PCA9685_DEBUG
    Serial.print("Attempting to set freq ");
    Serial.println(freq);
  #endif
  // Range output modulation frequency is dependant on oscillator
  if (freq < 1)
    freq = 1;
  if (freq > 3500)
    freq = 3500; // Datasheet limit is 3052=50MHz/(4*4096)
  pwmFreq = freq;
  float prescaleval = ((_oscillator_freq / (freq * 4096.0)) + 0.5) - 1;
  if (prescaleval < PCA9685_PRESCALE_MIN)
    prescaleval = PCA9685_PRESCALE_MIN;
  if (prescaleval > PCA9685_PRESCALE_MAX)
    prescaleval = PCA9685_PRESCALE_MAX;
  uint8_t prescale = (uint8_t)prescaleval;

  #ifdef PCA9685_DEBUG
    Serial.print("Final pre-scale: ");
    Serial.println(prescale);
  #endif

  uint8_t oldmode1 = read8(_i2caddr1, PCA9685_MODE1);
  uint8_t oldmode2 = read8(_i2caddr2, PCA9685_MODE1);
  uint8_t newmode1 = (oldmode1 & ~MODE1_RESTART) | MODE1_SLEEP; // sleep
  uint8_t newmode2 = (oldmode2 & ~MODE1_RESTART) | MODE1_SLEEP;
  write8(_i2caddr1, PCA9685_MODE1, newmode1);                             // go to sleep
  write8(_i2caddr1, PCA9685_PRESCALE, prescale); // set the prescaler
  write8(_i2caddr1, PCA9685_MODE1, oldmode1);
  delay(5);
  write8(_i2caddr2, PCA9685_MODE1, newmode2);                             // go to sleep
  write8(_i2caddr2, PCA9685_PRESCALE, prescale); // set the prescaler
  write8(_i2caddr2, PCA9685_MODE1, oldmode2);
  delay(5);
  // This sets the MODE1 register to turn on auto increment.
  write8(_i2caddr1, PCA9685_MODE1, oldmode1 | MODE1_RESTART | MODE1_AI);
  write8(_i2caddr2, PCA9685_MODE1, oldmode2 | MODE1_RESTART | MODE1_AI);
  return;
}

uint16_t PCA9685Driver::getPin(uint8_t num) {
  uint16_t result;
  uint8_t address;
  if (num<16) address = _i2caddr1;
  else if (num<32){
    num-=16;
    address = _i2caddr2;
  }else return 0xFFFF;
  uint8_t onL = read8(address, PCA9685_LED0_ON_L + 4 * num);
  uint8_t onH = read8(address, PCA9685_LED0_ON_L + 4 * num + 1);
  uint8_t offL = read8(address, PCA9685_LED0_ON_L + 4 * num + 2);
  uint8_t offH = read8(address, PCA9685_LED0_ON_L + 4 * num + 3);
  uint16_t onF = onL | (onH<<8);
  uint16_t offF = offL | (offH<<8);
  if (onF > 0xFFF) result = (uint16_t)0xFFF; //if led is full on
  else if (offF > 0xFFF) result = 0; // if led is full off
  else if (offF>onF) result = offF - onF;
  else result = (uint16_t)0xFFF + offF - onF;
  return result;
}

void PCA9685Driver::setPin(uint8_t pin, uint16_t val) {
  #ifdef PCA9685_DEBUG
  Serial.print("PCA9685driver set pin "+String(pin)+" to "+String(val)+". ");
  #endif
  // Clamp value between 0 and 4095 inclusive.
  if(pin>31){
    Serial.println("###Wrong value### PCA9685Driver::setPin(pin="+String(pin)+")");
    return;
  }
  val = min(val, (uint16_t)0xFFF);
  if (val == 0xFFF) {
    // Special value for signal fully on.
    setPWM(pin, 0x1FFF, 0);
  } else if (val == 0) {
    // Special value for signal fully off.
    setPWM(pin, 0, 0x1FFF);
  } else {
    /*
    uint16_t onTime=127*pin;
    uint16_t offTime=onTime + val;
    if(offTime>(uint16_t)0xFFF) offTime-=0xFFF;*/
    uint16_t onTime=0;
    uint16_t offTime = val;
    setPWM(pin, onTime, offTime);
  }
  return;
}

void PCA9685Driver::allLedOff(void){
  uint8_t regValue=1<<4;
  write8(_i2caddr1, PCA9685_ALLLED_OFF_H, regValue);
  write8(_i2caddr2, PCA9685_ALLLED_OFF_H, regValue);
}
/////////////////
//PRIVATE METHODS
/////////////////

void PCA9685Driver::setPWM(uint8_t num, uint16_t onTime, uint16_t offTime) {
#ifdef PCA9685_DEBUG
  //Serial.print("Setting PWM " + String(num) + ": " + String(onTime) + "->" + String(offTime));
#endif
  uint8_t address;
  if (num<16) address = _i2caddr1;
  else if (num<32){
    num-=16;
    address = _i2caddr2;
  }else return;
  #ifdef PCA9685_DEBUG
  Serial.println("on->"+String(onTime)+", off->"+String(offTime));
  #endif
  _i2c->beginTransmission(address);
  _i2c->write(PCA9685_LED0_ON_L + 4 * num);
  _i2c->write(onTime);
  _i2c->write(onTime >> 8);
  _i2c->write(offTime);
  _i2c->write(offTime >> 8);
  _i2c->endTransmission();
  //delay(1);
  return;
}

void PCA9685Driver::reset(void) {
  write8(_i2caddr1, PCA9685_MODE1, MODE1_RESTART);
  delay(10);
  write8(_i2caddr2, PCA9685_MODE1, MODE1_RESTART);
  delay(10);
}

void PCA9685Driver::setOscillatorFrequency(uint32_t freq) {
  _oscillator_freq = freq;
}

//****************** Low level I2C interface 
uint8_t PCA9685Driver::read8(uint8_t i2cAddr, uint8_t addr) {
  _i2c->beginTransmission(i2cAddr);
  _i2c->write(addr);
  _i2c->endTransmission();

  _i2c->requestFrom((uint8_t)i2cAddr, (uint8_t)1);
  return _i2c->read();
}

void PCA9685Driver::write8(uint8_t i2cAddr, uint8_t addr, uint8_t d) {
  _i2c->beginTransmission(i2cAddr);
  _i2c->write(addr);
  _i2c->write(d);
  _i2c->endTransmission();
}

void PCA9685Driver::setOCHBit(void) {
  #ifdef PCA9685_DEBUG
    Serial.print("Attempting to OCH bit ");
  #endif

  uint8_t oldmode1 = read8(_i2caddr1, PCA9685_MODE2);
  uint8_t oldmode2 = read8(_i2caddr2, PCA9685_MODE2);
  uint8_t newmode1 = oldmode1 | MODE2_OCH;
  uint8_t newmode2 = oldmode2 | MODE2_OCH;
  write8(_i2caddr1, PCA9685_MODE2, newmode1);                             // go to sleep
  write8(_i2caddr2, PCA9685_MODE2, newmode2); // set the prescaler
  delay(5);
  
  return;
}

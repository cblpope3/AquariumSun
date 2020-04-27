#ifndef _HTML_PAGE
#define _HTML_PAGE

#include <Arduino.h>
#include "PWM.h"

String makeMainHtmlPage(int value);

extern LedController leds;
//String makeSettingsHtmlPage(void);

#endif

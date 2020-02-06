#pragma once
#include "global.h"

//#define DEBUG
//#define SIG_DEBUG

#define PIN1
#define PIN2
#define PIN3

#define HIGH(pin)
#define LOW(pin)
#define BLINK(pin)

#define SIG_ON(action, pin) action(pin)
#define SIG_OFF(action, pin)

#define LOG_ON(log)
#define LOG_OFF(log)

// Срабатывает если true
#define ASSERT(cond)

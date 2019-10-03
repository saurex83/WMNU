#include "TIC.h"


static uint8_t TIC_getCurrentTS(void);
static void (*TIC_TDMAShelduler)(uint8_t TS);
static void (*TIC_HW_Timer_IRQ)(uint16_t ticks);
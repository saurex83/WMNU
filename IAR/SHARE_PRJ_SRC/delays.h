
#pragma once
#include "stdint.h"

void TIM_init(void);
uint32_t TIM_getTimeUs();
uint32_t TIM_passedTimeUs(uint32_t time);
void TIM_delayUs(uint32_t delay);
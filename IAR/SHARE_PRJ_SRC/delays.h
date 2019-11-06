#pragma once
#include "stdint.h"

void TIM_init(void);
uint32_t TIM_getTimeUs();
uint32_t TIM_passedTimeUs(uint32_t time);
void TIM_delayUs(uint32_t delay);
uint16_t TIM_getMACTicks();
uint32_t TIM_MAC_NS(uint16_t start, uint16_t stop);
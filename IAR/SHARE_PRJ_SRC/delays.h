#pragma once
#include "stdint.h"

#define Tusec 1
#define Tmsec 1000
#define Tsec 1000000

/**
@brief Описание временных меток таймера
*/
typedef struct
{
  union
  {
    uint32_t timer;
    uint8_t  byte[8]; // тут походу ошибка. должно быть 4???
  };
} TimeStamp_s;

void TIM_init(void);
void TIM_TimeStamp(TimeStamp_s* timestamp);
uint32_t TIM_passedTime(TimeStamp_s* start, TimeStamp_s* stop);
void TIM_delay(uint32_t delay);
void TIM_copy(TimeStamp_s *src, TimeStamp_s *dst);
#pragma once
#include "alarm_timer.h"

/**
@brief Представление модели модуля
*/
#define TM_MODEL TM
struct TM{
  int a;
};


typedef char timeslot_t;
typedef char alarm_t;

void TM_SetAlarm(timeslot_t slot, alarm_t alarm);
void TM_ClrAlarm(timeslot_t slot, alarm_t alarm);
void TM_AdjustTime(nwtime_t time);
void TM_IRQ(nwtime_t time);


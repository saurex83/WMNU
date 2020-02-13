#pragma once
#include "alarm_timer.h"

/**
@brief Представление модели модуля
*/

typedef char timeslot_t;
typedef char alarm_t;

#define TM_MODEL TM
struct TM{
  char MODE;
  timeslot_t timeslot;
  nwtime_t time;
};


void TM_SetAlarm(timeslot_t slot, alarm_t alarm);
void TM_ClrAlarm(timeslot_t slot, alarm_t alarm);
void TM_IRQ(nwtime_t time);


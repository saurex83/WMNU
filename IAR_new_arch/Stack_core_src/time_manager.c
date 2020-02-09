#include "time_manager.h"
#include "model.h"
#include "action_manager.h"
#include "debug.h"
#include "action_manager.h"

/**
@file
*/

#define MAX_TIME_SLOTS 50

#define ACTIVE_INTERVAL (nwtime_t)327 // 9.979 мс
#define SLEEP_INTERVAL (nwtime_t)327  // 9.979 мс
#define UNACCOUNTED 68 // Остаток времени после 50ого интервала.
#define FULL_INTERVAL (ACTIVE_INTERVAL + SLEEP_INTERVAL)
#define SLOT_TO_NWTIME(slot) ((nwtime_t)(slot * FULL_INTERVAL))

static void SW_Init(void); 

module_s TM_MODULE = {ALIAS(SW_Init)};
 
//!< Список задач менеджера. Индекс - номер слота, значение-действие
static alarm_t ALARMS[MAX_TIME_SLOTS];

static void SW_Init(void){}; 


void TM_SetAlarm(timeslot_t slot, alarm_t alarm){
  ASSERT(slot < MAX_TIME_SLOTS);
  ALARMS[slot] |= alarm;
}

void TM_ClrAlarm(timeslot_t slot, alarm_t alarm){
  ASSERT(slot < MAX_TIME_SLOTS);
  ALARMS[slot] &= (~alarm);
}

void TM_AdjustTime(nwtime_t time){
  
}

void TM_IRQ(nwtime_t time){
  LOG_OFF("ALARM! %d", time);
  AM_Hot_start();
}
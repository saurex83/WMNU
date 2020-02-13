#include "time_manager.h"
#include "model.h"
#include "action_manager.h"
#include "debug.h"
#include "cpu.h"
#include "macros.h"

/**
@file
*/

#define MAX_TIME_SLOTS 50

#define ACTIVE_INTERVAL (nwtime_t)327 // 9.979 мс
#define SLEEP_INTERVAL (nwtime_t)327  // 9.979 мс
#define UNACCOUNTED 68 // Остаток времени после 50ого интервала.
#define FULL_INTERVAL (ACTIVE_INTERVAL + SLEEP_INTERVAL)
#define SLOT_TO_NWTIME(slot) ((nwtime_t)((slot) * FULL_INTERVAL)) 
#define NWTIME_TO_SLOT(nwtime) ((timeslot_t)((time)/FULL_INTERVAL))
 
//!< Список задач менеджера. Индекс - номер слота, значение-действие
static alarm_t ALARMS[MAX_TIME_SLOTS];


void TM_SetAlarm(timeslot_t slot, alarm_t alarm){
  ASSERT(slot < MAX_TIME_SLOTS);
  ALARMS[slot] |= alarm;
}

void TM_ClrAlarm(timeslot_t slot, alarm_t alarm){
  ASSERT(slot < MAX_TIME_SLOTS);
  ALARMS[slot] &= (~alarm);
}

static inline timeslot_t _inc_timeslot(timeslot_t slot){
  slot++;
  return (slot >= MAX_TIME_SLOTS) ? 0 : slot;
}

static inline timeslot_t _find_next_active(timeslot_t slot){
  slot = _inc_timeslot(slot);
  while (!slot && !ALARMS[slot])
    slot = _inc_timeslot(slot);
  return slot;
}

static void scheulder_next_alarm(nwtime_t time){
  timeslot_t slot = NWTIME_TO_SLOT(time);
  slot = _find_next_active(slot);
  AT_set_alarm(SLOT_TO_NWTIME(slot));
}

void TM_IRQ(nwtime_t time){
  if (MODEL.TM.MODE == 0){
    AT_enable(false);
    return;
  }
  
  ATOMIC_BLOCK_FORCEON{    
    LOG_OFF("ALARM! %d", time); 
    MODEL.TM.timeslot = NWTIME_TO_SLOT(time);
    MODEL.TM.time = time;
    scheulder_next_alarm(time);
    AM_Hot_start();
  };
}

static inline void mcu_sleep(void){
}

static inline void _clr_all(void){
  for_each_type(alarm_t, ALARMS, i)
    i = 0;
}

static void init(void){
  _clr_all();
  scheulder_next_alarm(0);
  AT_enable(true);
}

static void start_mode_1(void){
  AM_SW_Init();
  init();
  
  while (true){
    AM_Sleep();
    mcu_sleep();
    
    if (MODEL.TM.MODE == 0)
      return;
    
    AM_Wakeup();
    AM_Cold_start();    
  }  
}

static void start_mode_2(void){
  AM_SW_Init();
  init();
}

void Neocore_start(void){
  switch (MODEL.TM.MODE){
  case 0:
    return;
  case 1:
    start_mode_1();
  case 2:
    start_mode_2();
  default: 
    HALT("Incorrect MODEL.TM.MODE");
  }
}
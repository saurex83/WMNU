#include "time_manager.h"
#include "model.h"
#include "action_manager.h"
#include "debug.h"

/**
@file
*/

// Определения
// Локальные переменные
// Локальные функции
static void HW_Init(void);  
static void SW_Init(void); 
static void Sleep(void); 
static void Wakeup(void); 
static void Cold_Start(void); 
static void Hot_Start(void); 

module_s TM_MODULE = {ALIAS(HW_Init),ALIAS(SW_Init),ALIAS(Sleep),
  ALIAS(Wakeup),ALIAS(Cold_Start), ALIAS(Hot_Start)};

static void HW_Init(void){};  
static void SW_Init(void){}; 
static void Sleep(void){}; 
static void Wakeup(void){}; 
static void Cold_Start(void){}; 
static void Hot_Start(void){}; 




void TM_SetAlarm(timeslot_t slot, alarm_t alarm){
}

void TM_ClrAlarm(timeslot_t slot, alarm_t alarm){
}

void TM_AdjustTime(nwtime_t time){
}

void TM_IRQ(nwtime_t time){
  LOG_ON("ALARM! %d", time);
}
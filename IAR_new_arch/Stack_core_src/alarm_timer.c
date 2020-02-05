#include "alarm_timer.h"
#include "time_manager.h"
#include "action_manager.h"

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

module_s AT_MODULE = {ALIAS(HW_Init),ALIAS(SW_Init),ALIAS(Sleep),
  ALIAS(Wakeup),ALIAS(Cold_Start), ALIAS(Hot_Start)};

static void HW_Init(void){};  
static void SW_Init(void){}; 
static void Sleep(void){}; 
static void Wakeup(void){}; 
static void Cold_Start(void){}; 
static void Hot_Start(void){};

void AT_set_time(nwtime_t time){
}

void AT_set_alarm(nwtime_t alarm){
}

nwtime_t AT_time(void){
  return 0;
}

void AT_wait(nwtime_t time){
  TM_IRQ(time); // тест
}



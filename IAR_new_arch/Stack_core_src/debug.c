#include "debug.h"
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

module_s DBG_MODULE = {ALIAS(HW_Init),ALIAS(SW_Init),ALIAS(Sleep),
  ALIAS(Wakeup),ALIAS(Cold_Start), ALIAS(Hot_Start)};

static void HW_Init(void){};  
static void SW_Init(void){}; 
static void Sleep(void){}; 
static void Wakeup(void){}; 
static void Cold_Start(void){}; 
static void Hot_Start(void){};

void ddD()
{
  SIG_OFF(HIGH, PIN1);
}

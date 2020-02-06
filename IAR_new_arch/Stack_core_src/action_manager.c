#include "action_manager.h"
#include "modules_list.h"
#include "model.h"
#include "debug.h"

/**
@file
*/

/* Определения */
#define ITEMS_COUNT(name) (sizeof(name)/sizeof(module_s*))
#define RUNNER(array, fun) {\
for (char i = 0; i < ITEMS_COUNT(array); i++){\
    ASSERT(array[i]->fun == NULL);\
    array[i]->fun();\
    }\
};


/* Локальные переменные */
static module_s* COLD_MOD[] = COLD_MOD_DEF;
static module_s* HOT_MOD[] = HOT_MOD_DEF;
static module_s* HW_MOD[] = HW_MOD_DEF;
static module_s* SW_MOD[] = SW_MOD_DEF;
static module_s* POW_MOD[] = POW_MOD_DEF;

/* Локальные функции */

void AM_Cold_start(void){
  RUNNER(COLD_MOD, Cold_Start);
}

void AM_Hot_start(void){
  RUNNER(HOT_MOD, Hot_Start);  
}

void AM_HW_Init(void){
  RUNNER(HW_MOD, HW_Init);      
}

void AM_SW_Init(void){
  RUNNER(SW_MOD, SW_Init);
}
void AM_Sleep(void){
  RUNNER(POW_MOD, Sleep);
}
void AM_Wakeup(void){
  RUNNER(POW_MOD, Wakeup);
}


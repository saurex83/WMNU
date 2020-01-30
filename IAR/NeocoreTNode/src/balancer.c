#include "nwdebuger.h"
#include "frame.h"
#include "LLC.h"
#include "config.h"

/**
@file
@detail
*/

// Определения

// Локальные функции

static void TB_TimeAlloc_HNDL(void);

// Глобальные функции
void TB_Init(void);
void TB_Reset(void);
void TB_Receive_HNDL(frame_s *fr);

// Локальные переменные


void TB_Init(void){
  LLC_TimeAlloc(TB_TimeAlloc_HNDL);
}

void TB_Reset(void){
}

/**
@brief Рабочее время модуля
*/
static void TB_TimeAlloc_HNDL(void){
}

/**
@brief Обработка пакета 
*/
void TB_Receive_HNDL(frame_s *fr){
}
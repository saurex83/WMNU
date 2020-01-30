#include "nwdebuger.h"
#include "LLC.h"
#include "frame.h"
#include "route.h"

/**
@file
@detail
*/

// Определения

// Локальные функции
static void NP_Receive_HNDL(frame_s *fr);
static void NP_TimeAlloc_HNDL(void);

// Глобальные функции
void NP_Reset(void);
void NP_Init(void);

// Локальные переменные

void NP_Init(void){
  LLC_TimeAlloc(NP_TimeAlloc_HNDL);
  // ROUTE.h bind table NP_Receive_HNDL
  NP_Reset();
}

void NP_Reset(void){
}

/**
@brief Приняли пакет протокола соседей
*/
static void NP_Receive_HNDL(frame_s *fr){
}

/**
@brief Рабочее время модуля
*/
static void NP_TimeAlloc_HNDL(void){
}
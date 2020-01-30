#include "nwdebuger.h"
#include "frame.h"
#include "LLC.h"
#include "ip.h"

/**
@file
@detail
*/

// Определения

// Локальные функции
static void AUTH_Receive_HNDL(frame_s *fr);
static void AUTH_TimeAlloc_HNDL(void);

// Глобальные функции
void AUTH_Init(void);
void AUTH_Reset(void);

// Локальные переменные

void AUTH_Init(void){
  LLC_TimeAlloc(AUTH_TimeAlloc_HNDL);
  // IP.h bind table AUTH_Receive_HNDL
  AUTH_Reset();
}

void AUTH_Reset(void){
}

/**
@brief Обработка пакета авторизации
*/
static void AUTH_Receive_HNDL(frame_s *fr){
}

/**
@brief Рабочее время модуля
*/
static void AUTH_TimeAlloc_HNDL(void){
};

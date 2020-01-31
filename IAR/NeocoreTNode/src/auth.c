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
static void AUTH_TimeAlloc_HNDL(void);

// Глобальные функции
void AUTH_Init(void);
void AUTH_Reset(void);
void AUTH_Receive_HNDL(frame_s *fr);

// Локальные переменные

void AUTH_Init(void){
  LLC_TimeAlloc(AUTH_TimeAlloc_HNDL);
  AUTH_Reset();
}

void AUTH_Reset(void){
}

/**
@brief Обработка пакета авторизации
*/
void AUTH_Receive_HNDL(frame_s *fr){
}

/**
@brief Рабочее время модуля
*/
static void AUTH_TimeAlloc_HNDL(void){
};

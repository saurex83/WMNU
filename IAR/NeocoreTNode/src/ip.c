#include "nwdebuger.h"
#include "frame.h"
#include "route.h"
#include "config.h"

/**
@file
@detail
*/

// Определения

// Локальные функции
static void IP_Receive_HNDL(frame_s *fr);

// Глобальные функции
void IP_Init(void);
void IP_Reset(void);
void IP_Send(frame_s *fr);

// Локальные переменные

void IP_Init(void){
  // ROUTE.h bind table IP_Receive_HNDL
  IP_Reset();
}

void IP_Reset(void){
}

/**
@brief Отправка IP пакета
*/
void IP_Send(frame_s *fr){
}

/**
@brief Приняли пакет протокола IP
*/
static void IP_Receive_HNDL(frame_s *fr){
}
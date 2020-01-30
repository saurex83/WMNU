#include "nwdebuger.h"
#include "ip.h"
#include "frame.h"

/**
@file
@detail
*/

// Определения

// Локальные функции
static void TCP_Receive_HNDL(frame_s *fr);

// Глобальные функции
void TCP_Init(void);
void TCP_Reset(void);
void TCP_Send(frame_s *fr);

// Локальные переменные

void TCP_Init(void){
  // ip.h bind table TCP_Receive_HNDL
  TCP_Reset();
}

void TCP_Reset(void){
}

/**
@brief Отправка пакета UDP
*/
void TCP_Send(frame_s *fr){
}

/**
@brief Обработка приема пакета UDP
*/
static void TCP_Receive_HNDL(frame_s *fr){
}
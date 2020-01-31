#include "nwdebuger.h"
#include "ip.h"
#include "frame.h"

/**
@file
@detail
*/

// Определения

// Локальные функции


// Глобальные функции
void TCP_Init(void);
void TCP_Reset(void);
void TCP_Send(frame_s *fr);
void TCP_Receive_HNDL(frame_s *fr);

// Локальные переменные

void TCP_Init(void){
  TCP_Reset();
}

void TCP_Reset(void){
}

/**
@brief Отправка пакета TCP
*/
void TCP_Send(frame_s *fr){
}

/**
@brief Обработка приема пакета TCP
*/
void TCP_Receive_HNDL(frame_s *fr){
}
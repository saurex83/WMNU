#include "nwdebuger.h"
#include "ip.h"
#include "frame.h"

/**
@file
@detail
*/

// Определения

// Локальные функции
static void UDP_Receive_HNDL(frame_s *fr);

// Глобальные функции
void UDP_Init(void);
void UDP_Reset(void);
void UDP_Send(frame_s *fr);

// Локальные переменные

void UDP_Init(void){
  // ROUTE.h bind table UDP_Receive_HNDL
  UDP_Reset();
}

void UDP_Reset(void){
}

/**
@brief Отправка пакета UDP
*/
void UDP_Send(frame_s *fr){
}

/**
@brief Обработка приема пакета UDP
*/
static void UDP_Receive_HNDL(frame_s *fr){
}
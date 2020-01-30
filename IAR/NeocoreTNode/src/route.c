#include "nwdebuger.h"
#include "ethernet.h"
#include "config.h"
#include "balancer.h"
#include "frame.h"
#include "LLC.h"

/**
@file
@detail
*/

// Определения

// Локальные функции
static void RP_Receive_HNDL(frame_s *fr);
static void RP_TimeAlloc_HNDL(void);

// Глобальные функции
void RP_Init(void);
void RP_Reset(void);
void RP_Send_GW(frame_s *fr);
void RP_Send(frame_s *fr);
void RP_SendRT_GW(frame_s *fr);
void RP_SendRT_RT(frame_s *fr);

// Локальные переменные



void RP_Init(void){
  LLC_TimeAlloc(RP_TimeAlloc_HNDL);
  ETH_SetRXCallback(RP_Receive_HNDL);
  RP_Reset();
}

void RP_Reset(void){
}

/**
@brief Отправка пакета на шлюз от текущего узла
*/
void RP_Send_GW(frame_s *fr){
}

/**
@brief Отправка без модификации данных, как есть
*/
void RP_Send(frame_s *fr){
}

/**
@brief Отправка пакета с маршрутизацией к шлюзу от другого узла.
 Процесс ретрансляции чущих данных. 
*/
void RP_SendRT_GW(frame_s *fr){
}

/**
@brief Отправка пакета с маршрутизацией от шлюза к другому узлу.
 Шлюз кому то передает данные, мы используюя таблицу маршрутизации
 перепосылаем пакет.
*/
void RP_SendRT_RT(frame_s *fr){
}

/**
@brief Обработчик принятого пакета уровня ethernet
*/
static void RP_Receive_HNDL(frame_s *fr){
  
  
  TB_Receive_HNDL(fr);
}

/**
@brief Рабочее время модуля
*/
static void RP_TimeAlloc_HNDL(void){
}
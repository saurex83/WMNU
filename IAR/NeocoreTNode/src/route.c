#include "nwdebuger.h"
#include "ethernet.h"
#include "config.h"
#include "balancer.h"
#include "frame.h"
#include "LLC.h"
#include "mem.h"

/**
@file
@detail
*/

// Определения
#define PID_BIND_TABLE_SIZE 10 //!< Максимальное кол-в протоколов PID

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
void RP_Bind_PID(uint8_t pid, void (*fn)(frame_s *fr));

// Локальные переменные
typedef struct{
    void (*fn)(frame_s *fr);
    uint8_t pid;
} pid_bind_s;

static pid_bind_s PID_BIND_TABLE[PID_BIND_TABLE_SIZE];

void RP_Init(void){
  LLC_TimeAlloc(RP_TimeAlloc_HNDL);
  ETH_SetRXCallback(RP_Receive_HNDL);
  RP_Reset();
}

void RP_Reset(void){
  // Таблицу связей не уничтожать
//  for (uint8_t i = 0; i < PID_BIND_TABLE_SIZE; i++){
//    PID_BIND_TABLE[i].fn == NULL;
//    PID_BIND_TABLE[i].pid = 0;
//  }     
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
@brief Регистрация обработчика PID
*/
void RP_Bind_PID(uint8_t pid, void (*fn)(frame_s *fr)){
  for (uint8_t i = 0; i < PID_BIND_TABLE_SIZE; i++){
    if (PID_BIND_TABLE[i].fn == NULL){
      PID_BIND_TABLE[i].fn = fn;
      PID_BIND_TABLE[i].pid = pid;
    }     
  }
}

/**
@brief Обработчик принятого пакета уровня ethernet
*/
static void RP_Receive_HNDL(frame_s *fr){
 //TODO Обнавление таблиц маршрутизации!
  bool found = false;
  for (uint8_t i = 0; i < PID_BIND_TABLE_SIZE; i++){
    if (PID_BIND_TABLE[i].pid == fr->meta.PID){
      found = true;
      PID_BIND_TABLE[i].fn(fr);
      break;
    }
  }
  
  // Нет обработчика нужного PID
  if (!found){
    frame_delete(fr);
    return;
  }
  // Балансеру достаточно метаданных. Если передавать пакет,
  // то становится вопрос а кто его удалять должен?
  meta_s meta;
  re_memcpy(&meta, &fr->meta, META_S_SIZE);
  TB_Receive_HNDL(&meta);
}

/**
@brief Рабочее время модуля
*/
static void RP_TimeAlloc_HNDL(void){
}
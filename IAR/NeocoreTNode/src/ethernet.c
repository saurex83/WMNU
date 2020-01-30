#include "nwdebuger.h"
#include "frame.h"
#include "Net_frames.h"
#include "LLC.h"
#include "config.h"

/**
@file
@detail
*/

// Определения

// Локальные функции
static void ETH_Receive_HNDL(frame_s *fr);

// Глобальные функции
void ETH_Init(void);
void ETH_Reset(void);
void ETH_Send(frame_s *fr);
void ETH_SetRXCallback(void (*fn)(frame_s *fr));

// Локальные переменные
static void (*ETH_Receive_CB)(frame_s *fr);


void ETH_Init(void){
  LLC_SetRXCallback(ETH_Receive_HNDL);
  ETH_Reset();
}

void ETH_Reset(void){
}

/**
@brief Установить обработчик пакета
*/
void ETH_SetRXCallback(void (*fn)(frame_s *fr)){
  ASSERT(fn != NULL);  
  ETH_Receive_CB = fn;
}

/**
@brief Обработчик принятого пакета
*/
static void ETH_Receive_HNDL(frame_s *fr){
/* 
Выделяем из принятого пакета заголовок ETH
Валидируем заголовок
Заполняем мета данные
Выбрасываем заголовок ETH
Передаем обработчику ETH_Receive_CB()
*/
}

/**
@brief Заполнение и отправка пакета
*/
void ETH_Send(frame_s *fr){
  
  bool res= LLC_AddTask(fr); // Если есть место, пакет добавится.
  if (!res)
    frame_delete(fr);
}
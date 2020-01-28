#include "LLC.h"
#include "nwdebuger.h"

/**
@file Буфер приемника пакетов.
@detail Буфер используется шлюзом для временого хранения принятых пакетов
 перед передачей компьютеру по уарт.
*/

// Определения
#define RX_BUFF_SIZE 10 //!< Количество пакетов для хранения 

// Глобальные функции
void RXB_Init(void);
void RXB_Reset(void);
uint8_t RXB_frame_count(void);
frame_s* RXB_get_frame(void);
void RXB_del_frame(frame_s *fr);

// Статические функции
static void RXB_Callback(frame_s *fr);
static void Delete_frame(uint8_t index);

// Локальные переменные
frame_s* FRAME_POOL[RX_BUFF_SIZE]; //!< Хранилище указателей на пакетов. NULL - нет пакета

/**
@brief Первоначальная настройка
*/
void RXB_Init(void){
  LLC_SetRXCallback(RXB_Callback);
  RXB_Reset();
}

/**
@brief Сброс всех данных и настройка
*/
void RXB_Reset(void){
  for (uint8_t i = 0; i < RX_BUFF_SIZE; i++)
    Delete_frame(i);
}

/**
@brief Количество принятых пакетов в буфере
@return Количество пакетов
*/
uint8_t RXB_frame_count(void){
  uint8_t cnt = 0;
  for (uint8_t i = 0; i < RX_BUFF_SIZE; i++)
    if (FRAME_POOL[i] != NULL)
      cnt ++;
  return cnt;
}

/**
@brief Возращает указатель на принятый пакет
*/
frame_s* RXB_get_frame(void){
  frame_s *rx_frame = NULL;

  for (uint8_t i = 0; i < RX_BUFF_SIZE; i++)
    if (FRAME_POOL[i] != NULL){
      rx_frame = FRAME_POOL[i];
      break;
    }
  return rx_frame;
}

/**
@brief Удаляет пакет по указателю
*/
void RXB_del_frame(frame_s *fr){
  int index = -1;
  
  for (uint8_t i = 0; i < RX_BUFF_SIZE; i++)
    if (FRAME_POOL[i] == fr){
      index = i;
      break;
    }    
  
  ASSERT(index > 0);
  Delete_frame(index);
}

/**
@brief Удаляет пакет из RX_BUFF_SIZE по индексу
@param[in] индекс пакета в массиве RX_BUFF_SIZE
*/
static void Delete_frame(uint8_t index){
  ASSERT(index < RX_BUFF_SIZE);
  if (FRAME_POOL[index] != NULL){
      frame_delete(FRAME_POOL[index]);
      FRAME_POOL[index] = NULL;
  }
}

/**
@brief Обработчик принятого пакета
@detail Вставляет принятые пакеты в буфере, если места нет то размещает
 по индексу 0.
*/
static void RXB_Callback(frame_s *fr){
  
  for (uint8_t i = 0; i < RX_BUFF_SIZE; i++)
    if (FRAME_POOL[i] == NULL){
      FRAME_POOL[i] = fr;
      return;
    }
  
  FRAME_POOL[0] = fr;
}
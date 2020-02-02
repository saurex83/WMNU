#include "nwdebuger.h"
#include "frame.h"
#include "Net_frames.h"
#include "LLC.h"
#include "config.h"
#include "basic.h"

/**
@file
@detail
*/

// Определения

// Локальные функции
static void ETH_Receive_HNDL(frame_s *fr);
static bool frame_filter(frame_s *fr);

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
@brief Проверка пакета на удолетворения фильтрам
*/
static bool frame_filter(frame_s *fr){
  ETH_LAY *eth_header = (ETH_LAY*)fr->payload;
  
  // Фильтр 1: по размеру кадра
  if (fr->len < ETH_LAY_SIZE)
    return false;
  
  // Фильтр 2: по версии протокола
  if (eth_header->ETH_T.bits.ETH_VER != HEADER_ETH_VER)
    return false;

  // Фильтр 3: по идентификатору сети
  if (eth_header->NETID!= CONFIG.panid)
    return false;
  
  // Фильтр 4: по адресу получателю
  if (eth_header->NDST != 0xffff )
    if (eth_header->NDST != CONFIG.node_adr)
      return false;
  
  return true;
}

/**
@brief Обработчик принятого пакета
*/
static void ETH_Receive_HNDL(frame_s *fr){
  // Проверяем пакет
  if (!frame_filter(fr)){
    frame_delete(fr);
    return;
  }

  // Заполняем метаданные
  ETH_LAY *eth_header = (ETH_LAY*)fr->payload;
  fr->meta.NDST = eth_header->NDST;
  fr->meta.NSRC = eth_header->NSRC;
  fr->meta.PID = eth_header->ETH_T.bits.PID;
  
    // Отрезаем заголовок и передаем на обработку дальше
  frame_delHeader(fr, ETH_LAY_SIZE);
  if (ETH_Receive_CB != NULL)
    ETH_Receive_CB(fr);
  else
    frame_delete(fr); 
}

/**
@brief Заполнение и отправка пакета.
@brief Поля заполняются используя метаданные. NETID = CONFIG.panid
 ETH_VER = #define HEADER_ETH_VER
*/
void ETH_Send(frame_s *fr){
  ETH_LAY eth_header;
  eth_header.ETH_T.bits.PID = fr->meta.PID;
  eth_header.ETH_T.bits.UNUSED = 0;
  eth_header.ETH_T.bits.ETH_VER = HEADER_ETH_VER;
  eth_header.NETID = CONFIG.panid;
  eth_header.NDST = fr->meta.NDST;
  eth_header.NSRC = fr->meta.NSRC;
  // CH и TS заполняет отправитель
  // Добавляем заголовок
  frame_addHeader(fr, &eth_header, ETH_LAY_SIZE);
  
  bool res= LLC_AddTask(fr); // Если есть место, пакет добавится.
  if (!res)
    frame_delete(fr);
}
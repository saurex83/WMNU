#include "nwdebuger.h"
#include "frame.h"
#include "route.h"
#include "config.h"
#include "Net_frames.h"
#include "udp.h"
#include "tcp.h"
#include "auth.h"
#include "neighnbor.h"

/**
@file
@detail
*/

// Определения

// Локальные функции
static void IP_Receive_HNDL(frame_s *fr);
static bool frame_filter(frame_s *fr);
static void ip_processing(frame_s *fr);
static void extract_metadata(frame_s *fr);

// Глобальные функции
void IP_Init(void);
void IP_Reset(void);
void IP_Send(frame_s *fr);

// Локальные переменные

void IP_Init(void){
  RP_Bind_PID(PID_IP, IP_Receive_HNDL);
  IP_Reset();
}

void IP_Reset(void){
}

/**
@brief Отправка IP пакета
*/
void IP_Send(frame_s *fr){
  IP_LAY ip_header;
  
  int etx = NP_GetETX(); // Некому передавать пакет
  if (etx == -1){
    frame_delete(fr);
    return;
  }
  
  ip_header.ETX = etx;
  fr->meta.ETX = etx;
  ip_header.FDST = 0x0000;
  fr->meta.FDST = 0x0000;
  ip_header.FSRC = CONFIG.node_adr;
  fr->meta.FSRC = CONFIG.node_adr;
  ip_header.IPP = fr->meta.IPP;
  fr->meta.PID = PID_IP;

  frame_addHeader(fr, &ip_header, IP_LAY_SIZE);
  RP_Send_GW(fr);
}


/**
@brief Фильтр пакетов
*/
static bool frame_filter(frame_s *fr){
  IP_LAY *ip_header = (IP_LAY*)fr->payload;
  
  // Фильтр 1: по размеру кадра
  if (fr->len < IP_LAY_SIZE)
    return false;
  
  // Фильтр 2: по типу протоколов верхнего уровня
  if (ip_header->IPP == IPP_UDP);
    return true;
    
  if (ip_header->IPP == IPP_TCP);
    return true;
  
  if (ip_header->IPP == IPP_AUTH);
    return true;

  return false;
}

/**
@brief Обработка принятого пакета узлом.
*/
static void ip_processing(frame_s *fr){
  frame_delHeader(fr, IP_LAY_SIZE);
  if (fr->meta.IPP == IPP_UDP)
    UDP_Receive_HNDL(fr);
  else if (fr->meta.IPP == IPP_TCP)
    TCP_Receive_HNDL(fr);
  else if (fr->meta.IPP == IPP_AUTH)
    AUTH_Receive_HNDL(fr);
}
  
/**
@brief Извлекает метаданные
*/
static void extract_metadata(frame_s *fr){
  IP_LAY *ip_header = (IP_LAY*)fr->payload;
  fr->meta.ETX = ip_header->ETX;
  fr->meta.FDST = ip_header->FDST;
  fr->meta.FSRC = ip_header->FSRC;
  fr->meta.IPP = ip_header->IPP;
}

/**
@brief Приняли пакет протокола IP. Пакет уже без ETH заголовка.
@detail Правила обработки : 
FDST=0xffff Пакет для всех. Обрабатываем.
FDST=0x0000 Пакет предназначен шлюзу. Пересылаем.
FDST=node_adr Пакет для нас. Обрабатываем.
FDST=все остальные. Маршрутизая по таблице маршрутов

*/
static void IP_Receive_HNDL(frame_s *fr){
  // Проверим пакет
  if (!frame_filter(fr)){
    frame_delete(fr);
    return;
  }  
  
  extract_metadata(fr);
 
  if (fr->meta.FDST == 0xffff) 
    ip_processing(fr);  // Пакет для всех улов. Обработка
  else if (fr->meta.FDST == 0x0000) 
    RP_SendRT_GW(fr); // Пакет отправить шлюзу. Маршрутизация
  else if (fr->meta.FDST == CONFIG.node_adr) 
    ip_processing(fr); // Пакет для нас. Обработка
  else if (fr->meta.FSRC == 0x0000)
    RP_SendRT_RT(fr); // От шлюза конкретному узлу. Маршрутизация
  else
    frame_delete(fr); // Хрен знает что за пакет. Сжечь ведьму!
}
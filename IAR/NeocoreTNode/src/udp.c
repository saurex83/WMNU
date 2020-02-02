#include "nwdebuger.h"
#include "ip.h"
#include "frame.h"
#include "Net_frames.h"

/**
@file
@detail
*/

// Определения
#define UDP_BIND_TABLE_SIZE 10 //!< Максимальное кол-в портов UDP

// Локальные функции


// Глобальные функции
void UDP_Init(void);
void UDP_Reset(void);
void UDP_Send(uint8_t port, frame_s *fr);
void UDP_Receive_HNDL(frame_s *fr);
void UDP_Bind(uint8_t port, void (*fn)(frame_s *fr));

// Локальные переменные
typedef struct{
    void (*fn)(frame_s *fr);
    uint8_t port;
} udp_bind_s;

static udp_bind_s UDP_BIND_TABLE[UDP_BIND_TABLE_SIZE];

void UDP_Init(void){
  UDP_Reset();
}

void UDP_Reset(void){
}

/**
@brief Регистрация обработчика UDP
*/
void UDP_Bind(uint8_t port, void (*fn)(frame_s *fr)){
  for (uint8_t i = 0; i < UDP_BIND_TABLE_SIZE; i++){
    if (UDP_BIND_TABLE[i].fn == NULL){
      UDP_BIND_TABLE[i].fn = fn;
      UDP_BIND_TABLE[i].port = port;
    }     
  }
}

/**
@brief Отправка пакета UDP
*/
void UDP_Send(uint8_t port, frame_s *fr){
  UDP_LAY udp_header;
  udp_header.port = port;
  fr->meta.IPP = IPP_UDP;
  frame_addHeader(fr, &udp_header, sizeof(udp_bind_s));
  IP_Send(fr);
}

/**
@brief Обработка приема пакета UDP
*/
void UDP_Receive_HNDL(frame_s *fr){
  UDP_LAY *udp_header = (UDP_LAY*)fr->payload;
  
  // Фильтр 1: по размеру кадра
  if (fr->len < UDP_LAY_SIZE){
    frame_delete(fr);
    return;
  }
 
  // Ищем обработчик порта, удаляем заголовок UDP 
  // и вызываем обработчик. Если обработчик незарегестрирован, то
  // удаляем принятый пакет
  for (uint8_t i = 0; i < UDP_BIND_TABLE_SIZE; i++){
    if (UDP_BIND_TABLE[i].port == udp_header->port){
      frame_delHeader(fr, UDP_LAY_SIZE);
      if (UDP_BIND_TABLE[i].fn != NULL)
        UDP_BIND_TABLE[i].fn(fr);
      else
        frame_delete(fr);
    }     
  }
}
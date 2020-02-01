#pragma once
#include "stdbool.h"
#include "stdint.h"

// Решил хранить все структуры пакетов в этом файле, не разбивая между слоями протоколов
// Так более наглядно и проще для изменений. Я не использую typedef enum в структурах кадров
// так как это может привести к проблеме размера enum. Она определяется компилятором

// Перечисления протоколов низкого уровня для ETH_H.ETH_T.PID
enum PID_e
{
   PID_IP = 1, PID_NP = 2
};

// Перечисления протоколов высокого уровня для IP_HEADER.IPP
enum IPP_e
{
  IPP_UDP = 0, IPP_TCP = 1, IPP_AUTH = 2
} ;

typedef struct
{  
  union 
  {
    uint8_t value;
    struct 
    {
      uint8_t PID:4;
      uint8_t UNUSED:1;
      uint8_t ETH_VER:3;
    } bits;
  } ETH_T;
  
  uint8_t NETID;
  uint16_t NDST;
  uint16_t NSRC;
} __attribute__((packed)) ETH_LAY;

#define ETH_LAY_SIZE sizeof(ETH_LAY) 

typedef struct 
{
  uint8_t ETX;
  uint16_t FDST;
  uint16_t FSRC;
  uint8_t IPP;
} __attribute__((packed)) IP_LAY;

#define IP_LAY_SIZE sizeof(IP_LAY)

typedef struct
{
  uint8_t ETX;
  uint8_t HOUR;
  uint8_t MIN;
  uint8_t SEC;
  uint16_t TIMER32K;
} __attribute__((packed)) SYNC_LAY;

#define SYNC_LAY_SIZE sizeof(SYNC_LAY)

typedef struct
{
  uint8_t port;
} __attribute__((packed)) UDP_LAY;

#define UDP_LAY_SIZE sizeof(UDP_LAY)
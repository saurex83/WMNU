#pragma once
#include "stdbool.h"
#include "stdint.h"

// Решил хранить все структуры пакетов в этом файле, не разбивая между слоями протоколов
// Так более наглядно и проще для изменений. Я не использую typedef enum в структурах кадров
// так как это может привести к проблеме размера enum. Она определяется компилятором

// Перечисления протоколов низкого уровня для ETH_H.ETH_T.PID
enum PID_e
{
  PID_SYNC = 0, PID_IP = 1, PID_NB = 2
};

// Перечисления протоколов высокого уровня для IP_HEADER.IPP
enum IPP_e
{
  IPP_UDP = 0, IPP_TCP = 1, IPP_AUTH = 2
} ;

typedef struct
{
  uint8_t LEN;
  
  union 
  {
    uint8_t value;
    struct 
    {
      uint8_t PID:4;
      uint8_t ACK:1;
      uint8_t ETH_VER:3;
    } bits;
  } ETH_T;
  
  uint8_t NETID;
  uint16_t NDST;
  uint16_t NSRC;
  uint32_t NONCE;
  uint8_t LENGTH;
} __attribute__((packed)) ETH_LAY;

#define ETH_LAY_SIZE sizeof(ETH_LAY) 

typedef struct //TODO Посмотреть в описании биты CRC, RSSI
{
  union 
  {
    uint8_t value;
    struct 
    {
      uint8_t undef:8; 
    } bits;
  } FCS1;
  
  union 
  {
    uint8_t value;
    struct 
    {
      uint8_t undef:8;
    } bits;
  } FCS2;
} __attribute__((packed)) ETH_F_t;

#define ETH_F_T_SIZE sizeof(ETH_F_t) 

typedef struct 
{
  uint8_t ETX;
  uint16_t FDST;
  uint16_t FSRC;
  uint8_t IPP;
  uint8_t LENGTH;
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
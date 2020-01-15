#pragma once
#include "stdbool.h"
#include "stdint.h"

/**
@brief Методики передачи пакета
*/
enum TX_METHODS {BROADCAST = 0, UNICAST = 1};

typedef struct 
{
   uint16_t TIMESTAMP; // В тактах сети
   int8_t RSSI_SIG; // Имя RSSI определено дефайном в ioCC2530.h
/* link quality. Calculate LQI = (CORR - a)b. Where a and b are found 
   empirically based on PER measurements as a function of the correlation 
   value. PER - packet error rate */
   int8_t LIQ;  
   uint8_t TS;
   uint8_t CH;
   bool ACK; // true если необходимо подтвеждение пакета
   uint8_t PID;
   uint16_t NDST;
   uint16_t NSRC;
   uint8_t ETX;
   uint16_t FDST;
   uint16_t FSRC;
   uint8_t IPP;
   uint8_t TX_METHOD;
   uint16_t SEND_TIME; // В тактах сети
} __attribute__((packed))  meta_s;

#define META_S_SIZE sizeof(meta_s) 

typedef struct 
{
  void *payload;
  uint8_t len;
  meta_s meta;
} __attribute__((packed)) frame_s;

#define FRAME_S_SIZE sizeof(frame_s) 

frame_s* frame_create(void);
void frame_delete(frame_s *fr);
uint8_t frame_len(frame_s *fr);
uint8_t frame_getCount(void);
void frame_delHeader(frame_s *fr, uint8_t len);
void frame_addHeader(frame_s *fr, void *src, uint8_t len);
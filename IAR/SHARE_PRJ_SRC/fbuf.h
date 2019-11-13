#pragma once
#include "stdbool.h"
#include "stdint.h"

/**
@brief Типы fbuf
*/
enum FBUF_TYPES {FB_ETH_LAY, FB_IP_LAY, FB_SYNC_LAY, FB_TRANSPORT_LAY,
                FB_RAW_LAY};

/**
@brief Структура fbuf
*/
typedef struct fbuf_s
{
  struct fbuf_s *next;
  void*  payload;
  uint8_t type;
  uint8_t len;
} __attribute__((packed))fbuf_s;

#define FBUF_S_SIZE sizeof(fbuf_s) 

fbuf_s* fbuf_create(uint8_t type, void* payload, uint8_t len);
void fbuf_delete(fbuf_s *fb);
void fbuf_chain(fbuf_s *h, fbuf_s *t);
fbuf_s* fbuf_next(fbuf_s *fb);
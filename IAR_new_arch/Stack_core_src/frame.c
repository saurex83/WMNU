#include "action_manager.h"
#include "frame.h"
#include "debug.h"
#include "global.h"
#include "mem_utils.h"
#include "mem_slots.h"

/**
@file Статическое хранение принятых пакетов
*/

static void SW_Init(void); 

module_s FR_MODULE = {ALIAS(SW_Init)};

static void SW_Init(void){ 
}; 

struct frame* FR_create(){
  return (struct frame*)SL_alloc();
};

void FR_delete(struct frame *frame){
  SL_free((char*)frame);
}

void FR_add_header(struct frame* frame ,void *head, char len){
  int new_len = frame->len + len;
  ASSERT( new_len < MAX_PAYLOAD_SIZE);
  
  // Сдвинем данные на размер вставки при необходимости
  if (frame->len != 0)
    MEMCPY(&frame->payload[len], frame->payload, len);
  
  // Скопируем новые данные
  MEMCPY(frame->payload, head, len);
  frame->len = new_len;
};

void FR_del_header(struct frame* frame, char len){
  ASSERT(len != NULL);
  ASSERT(frame->len >= len);
  MEMCPY(frame->payload, &frame->payload[len], len);
  
  #ifdef FRAME_FOOTER_DEL
  MEMSET(&frame->payload[len], 0, len);
  #endif
  
  frame->len = frame->len - len;;
}

int FR_busy(){
  return SL_busy();
}

int FR_available(){
  return SL_available();
};
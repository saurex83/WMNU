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
static int FRAME_COUNT = 0;

static void SW_Init(void){ 
  FRAME_COUNT = 0;
}; 

struct frame* FR_create(){
  FRAME_COUNT++;
  return (struct frame*)SL_alloc();
};

void FR_delete(struct frame *frame){
  FRAME_COUNT--;
  SL_free((char*)frame);
}

void FR_add_header(struct frame* frame ,void *head, char len){
  int new_len = frame->len + len;
  ASSERT( new_len < MAX_PAYLOAD_SIZE);
  
  // Сдвинем данные на размер вставки
  struct memcpy memcpy = {
    .src = frame->payload,
    .dst = &frame->payload[len],
    .len = frame->len
  };
  MEM_memcpy(&memcpy);
  // Скопируем новые данные
  memcpy.src = head;
  memcpy.dst = frame->payload;
  memcpy.len = len;
  MEM_memcpy(&memcpy);
  frame->len = new_len;
};

void FR_del_header(struct frame* frame, char len){
  ASSERT(frame->len >= len);
  int new_len = frame->len - len;
  frame->len = new_len;
  struct memcpy memcpy = {
    .src = &frame->payload[len],
    .dst = frame->payload,
    .len = new_len
  };  
  MEM_memcpy(&memcpy);  
}

int FR_busy(){
  return SL_busy();
}

int FR_available(){
  return SL_available();
};
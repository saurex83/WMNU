#include "action_manager.h"
#include "frame.h"
#include "debug.h"
#include "global.h"
#include "frame.h"

/**
@file Статическое хранение принятых пакетов
*/
#define FOR_EACH(ptr) for (                     \
     struct frame *ptr = FRAME_POOL;            \
     ptr <= &FRAME_POOL[FRAME_POOL_ITEMS_COUNT]; \
     ptr++)

#define MARK_CLEAR(slot) {slot->slot_flag = 0;}

enum slot_flags {
  TAKEN = 1<<0, SPACE = 1<<1, TX_READY = 1<<2, RX_READY = 1<<3
};

static void SW_Init(void); 
static frame_id user_space_create(void);
static frame_id irq_space_create(void);

module_s FR_MODULE = {ALIAS(SW_Init)};
static int USER_SPACE_INDEX;
static struct frame FRAME_POOL[FRAME_POOL_ITEMS_COUNT];

static void SW_Init(void){  
  FOR_EACH(slot)
    MARK_CLEAR(slot);
  USER_SPACE_INDEX = -1;
}; 


frame_id FR_Create(space sp){
  if (sp == USER_SPACE)
    return user_space_create();
  else if (sp == IRQ_SPACE)
    return irq_space_create();
  HALT("Space not exist");
  return -1;
}

/**
@brief
@detail Использует знание об элементе который ищет сосед
*/
static frame_id user_space_create(void){
  return -1;
}

static frame_id irq_space_create(void){
  //USER_SPACE_INDEX
  return -1;
}
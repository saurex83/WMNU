#include "action_manager.h"
#include "buffer.h"
#include "stddef.h"
#include "macros.h"
#include "model.h"
#include "debug.h"

#define MAX_SLOTS 50

static void SW_Init(void);
static void Cold_Start(void);
static void Hot_Start(void);

module_s LLC_MODULE = {ALIAS(SW_Init), ALIAS(Cold_Start), 
  ALIAS(Hot_Start)};

static char OPENSLOTS[MAX_SLOTS];

static void SW_Init(void){
  for_each_type(char, OPENSLOTS, slot)
    slot = false;
};

static void Cold_Start(void){};




static inline bool scan_tx_buffer_by_ts(struct frame *frame, timeslot_t ts){
  frame = NULL;
  void *cursor = BF_cursor_tx();
  if (!cursor)
    return false;
  do{
    frame = BF_content(cursor);
    if (frame->meta.TS == 0)
      HALT("Depricated slot 0");
    if (frame->meta.TS == ts)
      return true;
    cursor = BF_cursor_next(cursor);
  } while(cursor);
  return false;
}

static void receive(void){
}

static void transmite(void){
}

static void Hot_Start(void){
  timeslot_t ts = MODEL.TM.timeslot;
  if (ts == 0)
    return;
  
  if (OPENSLOTS[ts])
    receive();
  else 
    transmite();
};
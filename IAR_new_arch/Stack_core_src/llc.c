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

void LLC_open_slot(timeslot_t ts){
  ASSERT(ts > 1 && ts < MAX_SLOTS);
  OPENSLOTS[ts] = true;
}

void LLC_close_slot(timeslot_t ts){
  ASSERT(ts > 1 && ts < MAX_SLOTS);
  OPENSLOTS[ts] = false;
}

static void scheduler_tx(void){
  void *cursor = BF_cursor_tx();
  if (!cursor)
    return;
  
  struct frame *frame = NULL;
  while(!cursor){
    frame = BF_content(cursor);
    ASSERT(frame->meta.TS != 0);
    TM_SetAlarm(frame->meta.TS, 1);  
    cursor = BF_cursor_next(cursor);
  }
}

static void scheduler_rx(void){
  ASSERT(OPENSLOTS[0] == 0);
  for (char i = 1; i < MAX_SLOTS; i++)
    if (OPENSLOTS[i])
      TM_SetAlarm(i, 1);
    else
      TM_ClrAlarm(i, 1);
}

static void Cold_Start(void){
// Планировщик планирует на один суперфрейм в начале ts0
  if (MODEL.TM.timeslot != 0)
    return;
  
  scheduler_tx();
  scheduler_rx();
};

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
  // MAC получить
  struct frame *frame = NULL;
  if (!BF_push_rx(frame))
      HALT("BF_push_rx");
}

static void transmite(void){
  timeslot_t ts = MODEL.TM.timeslot;
  struct frame *frame = NULL;
  if (!scan_tx_buffer_by_ts(frame, ts))
    return;
  // MAC передать
  // void TM_ClrAlarm(timeslot_t slot, alarm_t alarm);
  // Нужно как то учитывать неудачных кол-во передач
  // frame->tx_attempts
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
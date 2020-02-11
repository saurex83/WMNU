#include "debug.h"
#include "buffer.h"

//bool BF_next_tx(struct frame* frame);
//bool BF_remove_tx(struct frame *frame);
//bool BF_remove_rx(struct frame *frame);
//bool BF_push_rx(struct frame *frame);
//bool BF_push_tx(struct frame *frame);

static void add_buf(void){
  struct frame *frame1 = FR_create();
  struct frame *frame2 = FR_create();
  bool res;
  
  res = BF_push_tx(frame1);
  if (res)
    printf("Pushed ok\r\n");
  else
    printf("Push err\r\n");
  
  res = BF_push_tx(frame2);
  if (res)
    printf("Pushed ok\r\n");
  else
    printf("Push err\r\n"); 
  
struct frame *cur_fr = NULL;  
  res = BF_next_tx(cur_fr);
  if (res)
    printf("In TX: %x", cur_fr);
  else
    printf("err");
  
  res = BF_next_tx(cur_fr);
  if (res)
    printf("In TX: %x", cur_fr);
  else
    printf("err");

  res = BF_next_tx(cur_fr);
  if (res)
    printf("In TX: %x", cur_fr);
  else
    printf("END");
  
}

void u_buffer(void){
  add_buf();
}
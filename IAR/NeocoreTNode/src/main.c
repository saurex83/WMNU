#include "manager.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "stdint.h"
#include "LLC.h"
#include "frame.h"
#include "MAC.h"
#include "TIC.h"
#include "frame.h"
#include "stdlib.h"

//  
// Собирать проект с ЗАКОМЕНТИРОВАННЫМ ключем GATEWAY в файле basic.h
#ifdef GATEWAY
#warning GATEWAY MUST UNDEFINE FOR NODE
#endif

void RX_frame(frame_s *fr);

frame_s* gen_frame(void){
  uint16_t payload;
  
  frame_s *fr = frame_create();
  fr->meta.TS = 34;
  fr->meta.CH = 20;
  fr->meta.TX_METHOD = BROADCAST;
  
  payload = rand();
  
  frame_addHeader(fr,&payload, sizeof(payload));
  return fr;
}

void main(void)
{
  neocore_hw_init();
  P1DIR = 0x13;
  network_discovery(100);
  LLC_SetRXCallback(RX_frame);
  MAC_OpenRXSlot(2, 15);
  
  while(1){
    
    if ((TIC_GetUptime() % 5) == 0)
      if (LLC_GetTaskLen() == 0){
        P1_1 = false;
        LLC_AddTask(gen_frame());
        P1_1 = true;
      }
    
    PCON = 1;
  }
}

void RX_frame(frame_s *fr){
  static bool state = false;
  P1_4 = !state;
  state = !state;
  frame_delete(fr);
}
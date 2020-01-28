#include "manager.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "stdint.h"
#include "LLC.h"
#include "frame.h"
#include "MAC.h"

void RX_frame(frame_s *fr);
//  
// Собирать проект с ЗАКОМЕНТИРОВАННЫМ ключем GATEWAY в файле basic.h
void main(void)
{
  neocore_hw_init();
  P1DIR = 0x13;
  network_discovery(100);
  LLC_SetRXCallback(RX_frame);
  MAC_OpenRXSlot(2, 15);
  while(1);
}

void RX_frame(frame_s *fr){
  static bool state = false;
  P1_4 = !state;
  state = !state;
  frame_delete(fr);
}
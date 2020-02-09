#include "action_manager.h"
#include "debug.h"

extern void u_test(void);
extern void u_alarm_timer(void);
extern void u_mem_utils(void);
extern void u_macros(void);
extern void u_mem_slots(void);
extern void u_frame(void);

void main(){
//  AM_HW_Init();
    AM_SW_Init();
  
  //u_test();
 // u_alarm_timer();
  //u_macros();
  //u_mem_utils(); // TODO красные зоны
  //u_mem_slots();
  u_frame();
  DBG_CORE_HALT();
}
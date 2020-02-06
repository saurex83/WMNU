#include "action_manager.h"
#include "debug.h"


extern void u_test(void);
extern void u_alarm_timer(void);
extern void u_mem_utils(void);

void main(){
  AM_HW_Init();
  
  //u_test();
 // u_alarm_timer();
  u_mem_utils(); // TODO красные зоны
  DBG_CORE_HALT();
}
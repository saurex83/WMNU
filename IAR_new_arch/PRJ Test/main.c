#include "action_manager.h"
#include "debug.h"


extern void u_test(void);
extern void u_alarm_timer(void);

void main(){
  AM_HW_Init();
  
  //u_test();
  u_alarm_timer();
  
  DBG_CORE_HALT();
}
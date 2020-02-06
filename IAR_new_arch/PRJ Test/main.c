#include "action_manager.h"
#include "debug.h"

void main(){
  AM_HW_Init();
  
  LOG_ON("HELLO");
  ASSERT(2>0);
  while(1);
}
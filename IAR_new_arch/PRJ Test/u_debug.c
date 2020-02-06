#include "debug.h"


void u_test(void){
  LOG_OFF("BAD")
  LOG_ON("HELLO");
  ASSERT(2<0);
  ASSERT(2>0);
}

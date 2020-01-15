#include "utest_suite.h"
#include "manager.h"
#include "nwdebuger.h"
#include "ioCC2530.h"

// Собирать проект без ключем GATEWAY в файле basic.h

void suite_NODE(void)
{
  SLEEPCMD |= 0x06; // Select PM2
   bool con = MG_Connect();
  if (con)
  {
    LOG_ON("Network sycronized");
  }  
  else
  {
    LOG_ON("Network connect faild");
  }
  
  while(1)
  {
    LOG_ON("Go PM2");
    PCON = 1;
  };
}
#include "utest_suite.h"
#include "manager.h"
#include "nwdebuger.h"

// Собирать проект без ключем GATEWAY в файле basic.h

void suite_NODE(void)
{
   bool con = MG_Connect();
  if (con)
  {
    LOG_ON("Network sycronized");
  }  
  else
  {
    LOG_ON("Network connect faild");
  }
  
  while(1);
}
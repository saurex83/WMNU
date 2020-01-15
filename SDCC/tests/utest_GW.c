#include "utest_suite.h"
#include "manager.h"
#include "nwdebuger.h"

// Собирать проект с ключем GATEWAY в файле basic.h



void suite_GW(void)
{
  bool con = MG_Connect();
  if (con)
  {
    LOG_ON("Network created\r\n");
  }  
  else
  {
    LOG_ON("Network creation failed\r\n");
  }
  
  while(1);
}
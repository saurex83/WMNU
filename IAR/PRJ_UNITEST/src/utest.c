#include "utest_suite.h"
#include "nwdebuger.h"
#include "utest_FItem.h"
#include "utest_FChain.h"

int main()
{
  nwDebugerInit();
  
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "Test log output\n");
  ASSERT(false, "Test assert output\n");

  //suite_FItem();
  suite_FChain();
  // Вызов функций тестирования
	//run_utest_framepart();

   

// Добавляем результаты тестов
	umsg_summary();
      while(1);
	return 0;
}

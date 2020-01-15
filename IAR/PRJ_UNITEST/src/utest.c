#include "utest_suite.h"
#include "nwdebuger.h"
#include "assert.h"

extern void suite_GW(void);
extern void suite_NODE(void);

int main()
{
  nwDebugerInit();
  utestSigTraceInit();
  LOG_ON("");
  LOG_ON("Test start");
 
  //suite_GW();
  suite_NODE();
// Добавляем результаты тестов
	umsg_summary();
 //     while(1);
	return 0;
}

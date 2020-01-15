#include "utest_suite.h"
#include "nwdebuger.h"
#include "assert.h"
#include "basic.h"

extern void suite_GW(void);
extern void suite_NODE(void);

int main()
{
  nwDebugerInit();
  utestSigTraceInit();
  LOG_OFF("");
  LOG_OFF("Test start");

#ifdef GATEWAY
  suite_GW();
#else
  suite_NODE();
#endif
// Добавляем результаты тестов
	umsg_summary();
 //     while(1);
	return 0;
}

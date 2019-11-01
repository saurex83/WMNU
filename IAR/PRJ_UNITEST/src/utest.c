#include "utest_suite.h"
#include "nwdebuger.h"

extern void suite_FItem(void);
extern void suite_FChain(void);
extern void suite_TIC(void);
extern void suite_TIC_HW(void);
extern void suite_MAC(void);
extern void suite_LLC(void);
extern void suite_NT_HW(void);
extern void suite_Frames(void);
extern void suite_RADIO_HW(void);
int main()
{
  nwDebugerInit();
  
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "Test log output\n");
  ASSERT(false, "Test assert output\n");

  // Вызов функций тестирования

 // suite_TIC();
 //  suite_TIC_HW();
   suite_RADIO_HW();
 // suite_Frames();
 // suite_FItem();
 // suite_FChain();
 // suite_LLC();
 // suite_NT_HW(); // Это аппаратный тест
	//run_utest_framepart();

// Добавляем результаты тестов
	umsg_summary();
 //     while(1);
	return 0;
}

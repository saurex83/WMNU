#include "utest_suite.h"
#include "nwdebuger.h"

extern void suite_fbuf(void);
extern void suite_TIC(void);
extern void suite_TIC_HW(void);
extern void suite_MAC(void);
extern void suite_LLC(void);
extern void suite_NT_HW(void);
extern void suite_Frames(void);
extern void suite_RADIO_HW(void);
extern void suite_delays_HW(void);
extern void suite_delays(void);
extern void suite_coder(void);
extern void suite_frame(void);
int main()
{
  nwDebugerInit();
  
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "Test log output\n");
  ASSERT(false, "Test assert output\n");

  // Вызов функций тестирования

 // suite_TIC();
 //  suite_TIC_HW();
   //suite_RADIO_HW();
 // suite_delays();
 // suite_delays_HW();
 // suite_coder();
 // suite_Frames();
  suite_fbuf();
  suite_frame();
 // suite_LLC();
 // suite_NT_HW(); // Это аппаратный тест
	//run_utest_framepart();

// Добавляем результаты тестов
	umsg_summary();
 //     while(1);
	return 0;
}

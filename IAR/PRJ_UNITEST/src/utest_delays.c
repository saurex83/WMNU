#include "delays.h"
#include "utest_suite.h"

static void alg_test(void)
{
  TIM_init();
  uint32_t start = TIM_getTimeUs();
  TIM_delayUs( (uint32_t)10 * (uint32_t)512);
  TIM_delayUs( (uint32_t)100 * (uint32_t)512);
  TIM_delayUs( (uint32_t)90 * (uint32_t)512);
  uint32_t passed = TIM_passedTimeUs(start);
  umsg("delays", "Delays test", passed == 102400);
  // TODO Тест с переполнением таймера
  
  start = TIM_getTimeUs();
  TIM_delayUs( (uint32_t)130560-512);
  TIM_delayUs( (uint32_t)100 * (uint32_t)512);
  passed = TIM_passedTimeUs(start);
  umsg("delays", "Delays test overflow", passed == 50688);
  
}

void suite_delays_HW(void)
{
  //umsg_line("delays module");
  //alg_test();
}


void suite_delays(void)
{
  umsg_line("delays module");
  alg_test();
}
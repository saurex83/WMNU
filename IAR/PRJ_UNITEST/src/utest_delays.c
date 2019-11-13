#include "delays.h"
#include "utest_suite.h"

static void alg_test(void)
{
  TIM_init();
  
  uint32_t passed;
  TimeStamp_s start, stop;
 
  TIM_TimeStamp(&start);
  TIM_TimeStamp(&stop);
  passed = TIM_passedTime(&start, &stop);
  umsg("delays", "TIM_passedTimeNs meas add op ", passed  < 10);
}

static void manual(void)
{
  TIM_init();
  
  uint32_t passed;
  TimeStamp_s start, stop;
  
  //Это ручная проверка работы модуля
  TIM_TimeStamp(&start);
  TIM_delay(500);
  TIM_TimeStamp(&stop);
  passed = TIM_passedTime(&start, &stop);
  umsg("delays", "TIM_passedTimeNs meas add op ", passed  < 10);
  
  TIM_TimeStamp(&start);
  TIM_delay(60*Tsec);
  TIM_TimeStamp(&stop);
  passed = TIM_passedTime(&start, &stop);
  umsg("delays", "TIM_passedTimeNs meas add op ", passed  < 10);
}

void suite_delays_HW(void)
{
  //umsg_line("delays module");
  manual();
}


void suite_delays(void)
{
  umsg_line("delays module");
  alg_test();
}
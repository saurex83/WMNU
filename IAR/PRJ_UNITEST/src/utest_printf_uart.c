#include "nwdebuger.h"
#include "utest_suite.h"
#include "delays.h"
#include "stdio.h"

void test_1(void)
{
  const char test_string1[] = "Hello world\n";
  const char test_string2[] = "Hello worldHello worldHello worldHello world\n";
  TimeStamp_s start, stop;
  
  TIM_init();
  TIM_TimeStamp(&start);
  printf(test_string1);
  TIM_TimeStamp(&stop);
  
  uint32_t passed = TIM_passedTime(&start, &stop);
  
  LOG_ON("String 1 len = %d. Print time = %lu us",
      sizeof(test_string1), passed);
  
  TIM_TimeStamp(&start);
  printf(test_string2);
  TIM_TimeStamp(&stop);
  
  passed = TIM_passedTime(&start, &stop);
  
  LOG_ON("String 2 len = %d. Print time = %lu us",
      sizeof(test_string2), passed);  
}

void suite_printf_uart(void)
{
  umsg_line("printf speed uart");
  test_1();
  
}
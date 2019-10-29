#include "utest_suite.h"
#include "NTMR.h"
#include "ioCC2530.h"

static uint8_t irq_counts;
NT_s *nt;

static void TimerHndl(uint16_t ticks)
{
  irq_counts++;
  nt->NT_SetCapture(0);
}

/** 
@brief Тестирование алгоритмов которые нельзя вынести из модуля 
*/
static void alg_test(void)
{
  irq_counts = 0;
  EA = 1;
  nt = NT_Create();
  nt->NT_SetEventCallback(TimerHndl);
  nt->NT_SetCapture(1000);
  nt->NT_IRQEnable(true);
  while (irq_counts<10);
  nt->NT_IRQEnable(false);
  
  nt->setTime(500);
  nt->NT_SetCapture(1000);
  // TODO в setcapture учитывать TOFFSET.
}

void suite_NT(void)
{
  umsg_line("NTMR module");
  alg_test();
}
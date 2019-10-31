#include "utest_suite.h"
#include "NTMR.h"
#include "ioCC2530.h"

NT_s *nt;

static bool finised;

static inline bool isNear(uint16_t A, uint16_t B, uint16_t x)
{
  if (A>B)
    A = A - B;
  else
    A = B - A;
  if (A<=x)
    return true;
  return false;
}


static void TimerHndl(uint16_t ticks)
{
  uint16_t irq_table[] = {0,1000, 2000, 25000,26000,27000};
  static uint8_t irq_num = 1; 
  static uint8_t loops = 3;
  
  volatile static uint16_t xx;
  xx = ticks;
  
  if (isNear(ticks, 0 , 3))
  {
    NT_SetCompare(1000);
    P0_0 = 1;
    P0_0 = 0;
  }
  else if (isNear(ticks, 1000 , 3))
  {
    NT_SetCompare(2000);
    P0_0 = 1;
    P0_0 = 0;
  }
  else if (isNear(ticks, 2000 , 3))
  {
    NT_SetCompare(25000);
    P0_0 = 1;
    P0_0 = 0;
  }  
  else if (isNear(ticks, 25000 , 3))
  {
    NT_SetCompare(26000);
    P0_0 = 1;
    P0_0 = 0;
  }
  else if (isNear(ticks, 26000 , 3))
  {
    NT_SetCompare(27000);
    P0_0 = 1;
    P0_0 = 0;
  }
  else if (isNear(ticks, 27000 , 3))
  {
    P0_0 = 1;
    P0_0 = 0;
    finised = true;
  }                    
}

/** 
@brief Тестирование алгоритмов которые нельзя вынести из модуля 
@deails Использует вывод P0_0 для индикации работы таймера
 Генерирует импульсы при каждом срабатывании. Нужно настроить логический
 анализатор на высокую частоту и работать с 1 каналом, иначе импульсы
 проглатывает.
*/
static void alg_test(void)
{
  EA = 1;

  P0DIR = 0xff;
  P0_0 = 0;
  
  nt = NT_Create();
  nt->NT_SetEventCallback(TimerHndl);
  
  // Тут поставить точку останова и запустить анализатор
  for (uint16_t i = 0 ; i < 1000; i++)
  {
    finised = false;
    nt->NT_SetCompare(0);
    while (!finised);
  }
  // Тут поставить точку останова
}

void suite_NT_HW(void)
{
  //umsg_line("NTMR module");
  alg_test();
}
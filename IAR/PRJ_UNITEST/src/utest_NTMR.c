#include "utest_suite.h"
#include "NTMR.h"
#include "ioCC2530.h"

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

  P0DIR = 0x01;
  P0_0 = 0;
  
  NT_Init();
  NT_SetEventCallback(TimerHndl);
  
  // Тут поставить точку останова и запустить анализатор
  // Вы должны увидеть 6 импульсов. Измерения по переднему фронту
  // Работаем на внутреннем RC генераторе
  // И0-И1 30.508мс
  // И1-И2 30.521мс
  for (uint16_t i = 0 ; i < 1000; i++)
  {
    finised = false;
    NT_SetCompare(0);
    while (!finised);
  }
  // Тут поставить точку останова
}

void suite_NT_HW(void)
{
  //umsg_line("NTMR module");
  alg_test();
}
#include "utest_suite.h"


static uint32_t TOFFSET, TIMER, NETWORK; //24 битные переменные
typedef union 
  {
    uint32_t val;
    uint8_t fild[4];
  } fild;


static void NT_setTime(void)
{
  fild  *timer, *network, *toffset;
  timer = (fild*)&TIMER;
  network = (fild*)&NETWORK;
  toffset = (fild*)&TOFFSET;
  
  // TOFFSET = NETWORK TIME - TIMER
  // тут нужно подумать как вычислять время в рамках 24 бит
  // TIMER - 24 бита, TOFFSET - 24 бита
  
  network->fild[2] = timer->fild[2]; // Расширяем сетевое время до 24 бит
  toffset->val = network->val - timer->val; // Вычисляем смещение
  toffset->val &= 0xFFFFFF; // Обрезаем лишнии биты
}

static uint16_t NT_GetTime(void)
{
  fild  *timer, *toffset;
  timer = (fild*)&TIMER;
  toffset = (fild*)&TOFFSET;
  
  uint32_t val = timer->val;
  // TOFFSET = NETWORK TIME - TIMER
  // NETWORK TIME = TIMER + TOFFSET
  val +=toffset->val;
  val &= 0x7FFF; // Приводим значения таймера к 0-32767
  return val;
}

/** 
@brief Тестирование алгоритмов которые нельзя вынести из модуля 
*/
static void alg_test(void)
{
  fild  *timer, *network, *toffset;
  timer = (fild*)&TIMER;
  network = (fild*)&NETWORK;
  toffset = (fild*)&TOFFSET;
 
  umsg_line("TEST SET/GET TIMER. NOT REAL FUNC.");
  // TOFFSET = NETWORK TIME - TIMER
  timer->val = 0;
  network->val = 10;
  NT_setTime();
  umsg("NTMR", "Set time test 1", toffset->val == 10);
 
  timer->val = 0;
  network->val = 32767;
  NT_setTime();
  umsg("NTMR", "Set time test 2", toffset->val == 32767);
  
  timer->val = 32000;
  network->val = 32767;
  NT_setTime();
  umsg("NTMR", "Set time test 3", toffset->val == 767); 
  
  timer->val = 1;
  network->val = 0;
  NT_setTime();
  umsg("NTMR", "Set time test 4", toffset->val == 0xffffff);
  
  timer->val = 0;
  network->val = 0;
  NT_setTime();
  umsg("NTMR", "Set time test 5", NT_GetTime() == 0); 
  
  timer->val = 0;
  network->val = 32767;
  NT_setTime();
  umsg("NTMR", "Set time test 6", NT_GetTime() == 32767);  
 
  timer->val = 0xFFFFFF;
  network->val = 32767;
  NT_setTime();
  umsg("NTMR", "Set time test 7", NT_GetTime() == 32767);
  
  timer->val = 0xFFFFFF;
  network->val = 32767;
  NT_setTime();
  umsg("NTMR", "Set time test 8", NT_GetTime() == 32767);
  
  timer->val = 0xFFFFFF;
  network->val = 0;
  NT_setTime();
  umsg("NTMR", "Set time test 9", NT_GetTime() == 0);
  
  timer->val = 32767;
  network->val = 0;
  NT_setTime();
  umsg("NTMR", "Set time test 10", toffset->val == 0xff8001);
}

void suite_NT(void)
{
  umsg_line("NTMR module");
  alg_test();
}
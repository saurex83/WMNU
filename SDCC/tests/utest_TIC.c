#include "utest_suite.h"
#include "TIC.h"
#include "NTMR.h"
#include "ioCC2530.h"

static void RXC(uint8_t TS)
{
  P0_1 = 1;
  P0_1 = 0;
}

static void TXC(uint8_t TS)
{
  P0_4 = 1;
  P0_4 = 0;
}

static void SEC(uint8_t TS)
{
  if (TS == 0)
  {
    P0_0 = 1;
    P0_0 = 0;
  }
}

/**
@details Аппаратный тест TIC. Выводит сигналы
на выводы P0_0 завершение нулевого слота. P0_1 прием, P0_4 передача.
Тайм слот 11 показывает приоритет передачи над приемом.
*/
static void test_hw(void)
{
  NT_Init();
  TIC_Init();
  EA = 1;
  P0DIR = 0xff;
  P0_0 = 0;
  P0_1 = 0;
  P0_2 = 0;
  
  TIC_SetRXCallback(RXC);
  TIC_SetTXCallback(TXC);
  TIC_SetSECallback(SEC);
  
  TIC_SetRXState(10, true);
  TIC_SetRXState(11, true);
  TIC_SetRXState(20, true);
  TIC_SetRXState(21, true);
  
  TIC_SetTXState(5, true);
  TIC_SetTXState(11, true);
 
  TIC_SetRTC(10000);
  TIC_SetNonce(5000);
  volatile uint32_t rtc, ut, nonce;
  while(true)
  {
    // тут можно остановить программу и посмотреть работу таймеров
    // rtc больше на 10000 чем uptime, nonce больше на 5000
    ut = TIC_GetUptime(); 
    rtc = TIC_GetRTC();
    nonce = TIC_GetNonce();
  }
}

void suite_TIC_HW(void)
{
  test_hw();
}
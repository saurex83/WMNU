#include "utest_suite.h"
#include "TIC.h"
#include "NTMR.h"

#define DAILY_SEC 86400
#define TS_ACTIVE (uint16_t)327 // 9.979 мс
#define TS_SLEEP (uint16_t)327  // 2.075 мс
#define TS_UNACCOUNTED 68
#define FULL_SLOT  (TS_ACTIVE+TS_SLEEP)

struct {
  bool nt_callback_set;
  uint16_t set_time;
  uint16_t set_capture;
  bool irq_enable;
  uint16_t get_time;
  uint8_t TS;
  bool RX_hndl;
  bool TX_hndl;
  bool SE_hndl;
  void (*nt_irq)(uint16_t tics);
} results;

// Моки для NTMR
static void mock_RX_hndl(uint8_t TS)
{
  results.RX_hndl = true;
  results.TS = TS;
}

static void mock_TX_hndl(uint8_t TS)
{
  results.TX_hndl = true;
  results.TS = TS;
}

static void mock_SE_hndl(uint8_t TS)
{
  results.SE_hndl = true;
  results.TS = TS;
}

static void mock_NT_SetEventCallback(void (*fn)(uint16_t ticks))
{
  results.nt_callback_set = true;
  results.nt_irq = fn;
};

static bool mock_NT_SetTime(uint16_t ticks)
{
  results.set_time = ticks;
  return true;
}

static bool mock_NT_SetCapture(uint16_t ticks)
{
  results.set_capture = ticks;
  return true;
}

static void mock_NT_IRQEnable(bool state)
{
  results.irq_enable = state;
}

static uint16_t mock_NT_GetTime(void)
{
  return results.get_time;
}

static void create_test(void)
{
  results.nt_callback_set = false;
  results.set_time = 0;
  results.set_capture = 0;
  results.irq_enable = false;
  results.get_time = 0;
  
  // Привязываем моки
  NT_s *nt = NT_Create();
  nt->NT_SetEventCallback = mock_NT_SetEventCallback;
  nt->NT_SetTime = mock_NT_SetTime;
  nt->NT_SetCapture = mock_NT_SetCapture;
  nt->NT_IRQEnable = mock_NT_IRQEnable;
  nt->NT_GetTime = mock_NT_GetTime;
  
  TIC_s *tic = TIC_Create(nt);
  umsg("TIC", "TIC created", tic != NULL);
  umsg("TIC", "NT_SetEventCallback called", results.nt_callback_set == true);
  
  tic->TIC_SetTimer(10);
  umsg("TIC", "TIC_SetTimer", results.set_time == 10);
  
  results.get_time = 7;
  uint16_t tm = tic->TIC_GetTimer();
  umsg("TIC", "TIC_GetTimer ", tm == 7);

  uint8_t up = tic->TIC_GetUptime();
  umsg("TIC", "TIC_GetUptime = 0", up == 0);
  
  uint8_t rt = tic->TIC_GetRTC();
  umsg("TIC", "TIC_GetRTC = 0", rt == 0);
  
  bool setrctres = tic->TIC_SetRTC(DAILY_SEC);
  umsg("TIC", "TIC_SetRTC 86400 sec", setrctres == false);
  
  setrctres = tic->TIC_SetRTC(DAILY_SEC-1);
  uint32_t grtc = tic->TIC_GetRTC();
  umsg("TIC", "TIC_SetRTC = 86399", grtc == (DAILY_SEC-1));
  umsg("TIC", "TIC_SetRTC = 86399 retval true", setrctres == true);
  
// Тестируем SetTXState, GetTXState
// Тестируем SetRXState, GetRXState
  bool txstat=true;
  for (uint8_t i = 0;i < 50; i++)
  {
    tic->TIC_SetTXState(i, true);
    if (tic->TIC_GetTXState(i) == false)
      txstat = false;
  }
  umsg("TIC", "TIC_SetTXState, TIC_GetTXState state=true", txstat);
  
  txstat=true;
  for (uint8_t i = 0;i < 50; i++)
  {
    tic->TIC_SetTXState(i, false);
    if (tic->TIC_GetTXState(i) == true)
      txstat = false;
  }
  umsg("TIC", "TIC_SetTXState, TIC_GetTXState state=false", txstat);
  
 bool rxstat=true;
  for (uint8_t i = 0;i < 50; i++)
  {
    tic->TIC_SetRXState(i, true);
    if (tic->TIC_GetRXState(i) == false)
      rxstat = false;
  }
  umsg("TIC", "TIC_SetRXState, TIC_GetRXState state=true", rxstat);
  
  rxstat=true;
  for (uint8_t i = 0;i < 50; i++)
  {
    tic->TIC_SetRXState(i, false);
    if (tic->TIC_GetRXState(i) == true)
      rxstat = false;
  }
  umsg("TIC", "TIC_SetRXState, TIC_GetRXState state=false", rxstat);
  
  
// Имитируем работу таймера
  tic->TIC_SetRTC(0);
  tic->TIC_SetRXCallback(mock_RX_hndl);
  tic->TIC_SetTXCallback(mock_TX_hndl);
  tic->TIC_SetSECallback(mock_SE_hndl);
  
  results.SE_hndl = false;
  results.TS = 5;
// Убедимся что счет секунд активен  
  results.nt_irq(0);
  umsg("TIC", "TIC_GetRTC = 1", tic->TIC_GetRTC() == 1);
  umsg("TIC", "TIC_GetUptime = 1", tic->TIC_GetUptime() == 1);
  umsg("TIC", "SECallback active", results.SE_hndl);
  umsg("TIC", "SECallback TS = 0 ", results.TS ==0);

  // Иммитируем начало 10 слота
  results.TX_hndl = false;
  tic->TIC_SetTXState(10, true);
  results.nt_irq(10*FULL_SLOT);
  umsg("TIC", "TXCallback active", results.TX_hndl);
  umsg("TIC", "TXCallback TS = 10 ", results.TS ==10); 
  umsg("TIC", "TIC_GetUptime = 1 not incriment", tic->TIC_GetUptime() == 1);
  tic->TIC_SetTXState(10, false);
  
  // Иммитируем начало 49 слота
  results.RX_hndl = false;
  tic->TIC_SetRXState(49, true);
  results.nt_irq(49*FULL_SLOT);
  umsg("TIC", "RXCallback active", results.RX_hndl);
  umsg("TIC", "RXCallback TS = 49 ", results.TS ==49); 
  umsg("TIC", "TIC_GetUptime = 1 not incriment", tic->TIC_GetUptime() == 1);
  tic->TIC_SetRXState(49, false);
  
// Проверим планировщик
  tic->TIC_SetRXState(20, true);
  results.nt_irq(0);
  umsg("TIC", "TIC_TDMAShelduler set capture to slot 20",
       results.set_capture == 20*FULL_SLOT);
  
  TIC_Delete(tic);
  NT_Delete(nt);
}

void suite_TIC(void)
{
  umsg_line("TIC module");
  create_test();
}
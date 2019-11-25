#include "TIC.h"
#include "NTMR.h"
#include "stdlib.h"
#include "nwdebuger.h"
#include "delays.h"

/**
@file 
@brief 
@details
*/

// Публичные методы
void TIC_Init(void);

// Методы класса
void TIC_SetTimer(uint16_t ticks);
uint16_t TIC_GetTimer(void);
bool TIC_SetTXState(uint8_t TS, bool state);
bool TIC_SetRXState(uint8_t TS, bool state);
bool TIC_GetTXState(uint8_t TS);
bool TIC_GetRXState(uint8_t TS);
void TIC_SetRXCallback(void (*fn)(uint8_t TS));
void TIC_SetTXCallback(void (*fn)(uint8_t TS));
void TIC_SetSECallback(void (*fn)(uint8_t TS));
uint32_t TIC_GetUptime(void);
uint32_t TIC_GetRTC(void);
bool TIC_SetRTC(uint32_t RTC);
void TIC_SetNonce(uint32_t nonce);
uint32_t TIC_GetNonce(void);
uint32_t TIC_TimeUsFromTS0();
TimeStamp_s* TIC_GetTimeStampTS0(void);
 
// Приватные методы
static uint8_t TIC_getCurrentTS(uint16_t ticks);
static void TIC_TDMAShelduler(uint8_t TS);
static void TIC_HW_Timer_IRQ(uint16_t ticks);
static inline void set_capture_time(uint8_t TS);
static inline void Callback_execution(void (*fn)(uint8_t TS), uint8_t TS);
static inline void incrementTS(uint8_t *TS);
static void clocks_update(void);

// Переменные модуля

// Всего 50 слотов. (50 активных и 50 слотов сна)
// Кварц имеет 32768 тактов в секунду. После 100 слотов
// Остается 68 неучтеных тактов. Их нужно учитывать.
#define MAX_TS (uint8_t)50
#define TS_ACTIVE (uint16_t)327 // 9.979 мс
#define TS_SLEEP (uint16_t)327  // 2.075 мс
#define TS_UNACCOUNTED 68
// Целый цикл-это сумма времени активного периода и сна
#define FULL_SLOT  (TS_ACTIVE+TS_SLEEP)
#define NO_TIME_SLOT 0xFF

#define TS_RX (uint8_t)(1<<0)
#define TS_TX (uint8_t)(1<<1)
#define DAILY_SEC (uint32_t)86400
#define MAX_TICKS (uint16_t)32768

static uint32_t NODE_UPTIME = 0;
static uint32_t NODE_RTC = 0;
static uint32_t NODE_NONCE = 0;
static void (*RXCallback)(uint8_t TS);
static void (*TXCallback)(uint8_t TS);
static void (*SECallback)(uint8_t TS);
static uint8_t TSStateTable[MAX_TS];
static TimeStamp_s TimeStampTS0;


TimeStamp_s* TIC_GetTimeStampTS0(void)
{
  return &TimeStampTS0;
}
void TIC_Init(void)
{    
  // Устанавливаем обработчик прерываний таймера
  NT_SetEventCallback(TIC_HW_Timer_IRQ);
  // Запускаем процесс планировщика
  NT_SetCompare(0); 
  TIC_SetTimer(0);
  TIM_TimeStamp(&TimeStampTS0);
}

uint32_t TIC_TimeUsFromTS0()
{
  uint32_t passed;
  TimeStamp_s now;
  TIM_TimeStamp(&now);
  passed = TIM_passedTime(&TimeStampTS0, &now);
  return passed;
}

void TIC_SetTimer(uint16_t ticks)
{
  ASSERT_HALT(ticks < MAX_TICKS, "Ticks not in range");
  if (ticks >= MAX_TICKS)
    return ;
  
  NT_SetTime(ticks);
}

uint16_t TIC_GetTimer(void)
{
  return NT_GetTime();
}

bool TIC_SetTXState(uint8_t TS, bool state)
{
  if (TS>=MAX_TS)
  {
    return false;
  }
  ASSERT_HALT(TS<MAX_TS, "TS not in range");
  
  if (state)
    TSStateTable[TS] |= TS_TX;
  else
    TSStateTable[TS] &= ~TS_TX;
  
  return true;
}

bool TIC_SetRXState(uint8_t TS, bool state)
{
  if (TS>=MAX_TS)
  {
    return false;
  }
  ASSERT_HALT(TS<MAX_TS, "TS not in range");
  
  if (state)
    TSStateTable[TS] |= TS_RX;
  else
    TSStateTable[TS] &= ~TS_RX;
  
  return true;
}

bool TIC_GetTXState(uint8_t TS)
{
    if (TS>=MAX_TS)
  {
    return false;
  }
  ASSERT_HALT(TS<MAX_TS, "TS not in range");
  
  return (TSStateTable[TS] & TS_TX) ;
}

bool TIC_GetRXState(uint8_t TS)
{
    if (TS>=MAX_TS)
  {
    return false;
  }
  ASSERT_HALT(TS<MAX_TS, "TS not in range");
  
  return (TSStateTable[TS] & TS_RX) ;
}

void TIC_SetRXCallback(void (*fn)(uint8_t TS))
{
  ASSERT_HALT(fn != NULL, "Fn is NULL");
  RXCallback = fn;
}

void TIC_SetTXCallback(void (*fn)(uint8_t TS))
{
  ASSERT_HALT(fn != NULL, "Fn is NULL");
  TXCallback = fn;
}

void TIC_SetSECallback(void (*fn)(uint8_t TS))
{
  ASSERT_HALT(fn != NULL, "Fn is NULL");
  SECallback = fn;
}

uint32_t TIC_GetUptime(void)
{
  return NODE_UPTIME;
}

uint32_t TIC_GetRTC(void)
{
  return NODE_RTC;
}

bool TIC_SetRTC(uint32_t RTC)
{
  if (RTC >= DAILY_SEC)
    return false;
  
  NODE_RTC = RTC;
  return true;
}

static uint8_t TIC_getCurrentTS(uint16_t ticks)
{
  //uint16_t ticks = nt->NT_GetTime();
  // Вычисляем количество целых циклов
  uint8_t full_slot_num = ticks/FULL_SLOT;
  
  // Вычисляем остаток от деления. Остаток - смещение внутри
  // временного слота.
  uint16_t time_remainder = ticks % FULL_SLOT; 
  // Если значение внутри интервала 0..TS_ACTIVE. 
  if (time_remainder < TS_ACTIVE)
    return full_slot_num;
  
  return NO_TIME_SLOT;
}

static inline void incrementTS(uint8_t *TS)
{
  // Выбираем следующий тайм слот
  (*TS)++;
  if (*TS >= MAX_TS)
    *TS = 0;
}

static inline void set_capture_time(uint8_t TS)
{
  uint16_t ct = FULL_SLOT*(uint16_t)TS;
  
  LOG(MSG_OFF | MSG_INFO | MSG_TRACE, "Set compare = %d, TS = %d \r\n", ct, TS);
  // Установка прерывания на нужный слот
  NT_SetCompare(FULL_SLOT*(uint16_t)TS);
}

static void TIC_TDMAShelduler(uint8_t TS)
{
  // Устанавливает прерывание на ближайший активный слот
  // Если слот 0 ближе, то на него. 
  // Cлот - обработка секундных таймеров 
  
  // Ищем следующий активный слот или слот 0
  incrementTS(&TS);
  while ((TS != 0) && !TSStateTable[TS])  
    incrementTS(&TS); 
  
  set_capture_time(TS);
}

static inline void Callback_execution(void (*fn)(uint8_t TS), uint8_t TS)
{
  // Помошник вызова функций. Упрощает проверки
  ASSERT_HALT(fn != NULL, "Callback is NULL");
  if (fn == NULL)
    return;
  fn(TS);
}

static void TIC_HW_Timer_IRQ(uint16_t ticks)
{
  // TODO Обработка TS=0xFF
  // Номер текущего слота 
  uint8_t c_TS = TIC_getCurrentTS(ticks);
  
  // Обновляем часы NODE_RTC и NODE_UPTIME
  if (c_TS == 0)
  {
    TIM_TimeStamp(&TimeStampTS0); // Первым делом обновим отметку точного времени
    clocks_update();
  }
  // Если что то пошло не так и мы промахнулись мимо слота
  // запускаем планировщик заново
  if (c_TS == NO_TIME_SLOT)
  {
    TIC_TDMAShelduler(c_TS);
    LOG(MSG_ON | MSG_INFO | MSG_TRACE, "TS = 255 \r\n");
    return;
  }
    
  // Вызываем один из указанных обработчиков.
  // Передача имеет приоритет над приемом.
  if (TSStateTable[c_TS] & TS_TX) 
    Callback_execution(TXCallback, c_TS);
  else if (TSStateTable[c_TS] & TS_RX)
    Callback_execution(RXCallback, c_TS);
    
  // Вызываем обработчик завершения слота
  Callback_execution(SECallback, c_TS);
  
  // Запускаем планировщик таймера
  TIC_TDMAShelduler(c_TS);
}

static void clocks_update(void)
{
  NODE_UPTIME++;
  NODE_RTC++;
  NODE_NONCE++;
  if (NODE_RTC >= DAILY_SEC)
    NODE_RTC = 0;
}

void TIC_SetNonce(uint32_t nonce)
{
  NODE_NONCE = nonce;
}

uint32_t TIC_GetNonce(void)
{
  return NODE_NONCE;
}
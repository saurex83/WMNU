#include "TIC.h"
#include "stdlib.h"
#include "nwdebuger.h"

// Публичные методы
TIC_s* TIC_Create(NT_s* nt);
bool TIC_Delete(TIC_s *tic);

// Методы класса
static bool TIC_SetTimer(uint16_t ticks);
static uint16_t TIC_GetTimer(void);
static bool TIC_SetTXState(uint8_t TS, bool state);
static bool TIC_SetRXState(uint8_t TS, bool state);
static bool TIC_GetTXState(uint8_t TS);
static bool TIC_GetRXState(uint8_t TS);
static void TIC_SetRXCallback(void (*fn)(uint8_t TS));
static void TIC_SetTXCallback(void (*fn)(uint8_t TS));
static void TIC_SetSECallback(void (*fn)(uint8_t TS));
static uint32_t TIC_GetUptime(void);
static uint32_t TIC_GetRTC(void);
static bool TIC_SetRTC(uint32_t RTC);

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
// Целый цикл-это сума времени активного периода и сна
#define FULL_SLOT  (TS_ACTIVE+TS_SLEEP)
#define NO_TIME_SLOT 0xFF

#define TS_RX (uint8_t)(1<<0)
#define TS_TX (uint8_t)(1<<1)
#define DAILY_SEC (uint32_t)86400
#define MAX_TICKS (uint16_t)32768

static uint32_t NODE_UPTIME = 0;
static uint32_t NODE_RTC = 0;
static void (*RXCallback)(uint8_t TS);
static void (*TXCallback)(uint8_t TS);
static void (*SECallback)(uint8_t TS);
static bool TIC_CREATED = false;
static NT_s* TIC_nt;
static uint8_t TSStateTable[MAX_TS];

TIC_s* TIC_Create(NT_s* nt)
{
  if (TIC_CREATED)
  {
    ASSERT_HALT(TIC_CREATED == false, "TIC already created");
    return NULL;
  }
  
  ASSERT_HALT(nt != NULL, "NT must be set");
  TIC_nt = nt;
  
  TIC_s* tic = malloc(TIC_S_SIZE);
  ASSERT_HALT(nt != NULL, "Memory allocation fails");
  if (tic == NULL)
    return NULL;
  
  // Заполняем структуру указателей
  tic->TIC_SetTimer = TIC_SetTimer;
  tic->TIC_GetTimer = TIC_GetTimer;
  tic->TIC_SetTXState = TIC_SetTXState;
  tic->TIC_SetRXState = TIC_SetRXState;
  tic->TIC_GetTXState = TIC_GetTXState;
  tic->TIC_GetRXState = TIC_GetRXState;
  tic->TIC_SetRXCallback = TIC_SetRXCallback;
  tic->TIC_SetTXCallback = TIC_SetTXCallback;
  tic->TIC_SetSECallback = TIC_SetSECallback;
  tic->TIC_GetUptime = TIC_GetUptime;
  tic->TIC_GetRTC = TIC_GetRTC;
  tic->TIC_SetRTC = TIC_SetRTC;
  
  // Устанавливаем обработчик прерываний таймера
  nt->NT_SetEventCallback(TIC_HW_Timer_IRQ);
  
  TIC_CREATED = true;
  return tic;
}

bool TIC_Delete(TIC_s *tic)
{
  ASSERT_HALT(tic != NULL, "Cant free NULL TIC");
  if (tic == NULL)
    return false;
  
  free(tic);
  TIC_CREATED = false;
  return true;
}

static bool TIC_SetTimer(uint16_t ticks)
{
  ASSERT_HALT(ticks < MAX_TICKS, "Ticks not in range");
  if (ticks >= MAX_TICKS)
    return false;
  
  TIC_nt->NT_SetTime(ticks);
  return true;
}

static uint16_t TIC_GetTimer(void)
{
  return TIC_nt->NT_GetTime();
}

static bool TIC_SetTXState(uint8_t TS, bool state)
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

static bool TIC_SetRXState(uint8_t TS, bool state)
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

static bool TIC_GetTXState(uint8_t TS)
{
    if (TS>=MAX_TS)
  {
    return false;
  }
  ASSERT_HALT(TS<MAX_TS, "TS not in range");
  
  return (TSStateTable[TS] & TS_TX) ;
}

static bool TIC_GetRXState(uint8_t TS)
{
    if (TS>=MAX_TS)
  {
    return false;
  }
  ASSERT_HALT(TS<MAX_TS, "TS not in range");
  
  return (TSStateTable[TS] & TS_RX) ;
}

static void TIC_SetRXCallback(void (*fn)(uint8_t TS))
{
  ASSERT_HALT(fn != NULL, "Fn is NULL");
  RXCallback = fn;
}

static void TIC_SetTXCallback(void (*fn)(uint8_t TS))
{
  ASSERT_HALT(fn != NULL, "Fn is NULL");
  TXCallback = fn;
}

static void TIC_SetSECallback(void (*fn)(uint8_t TS))
{
  ASSERT_HALT(fn != NULL, "Fn is NULL");
  SECallback = fn;
}

static uint32_t TIC_GetUptime(void)
{
  return NODE_UPTIME;
}

static uint32_t TIC_GetRTC(void)
{
  return NODE_RTC;
}

static bool TIC_SetRTC(uint32_t RTC)
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
  // Установка прерывания на нужный слот
    TIC_nt->NT_SetCapture(FULL_SLOT*(uint16_t)TS);
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
    clocks_update();
  
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
  if (NODE_RTC >= DAILY_SEC)
    NODE_RTC = 0;
}


#include "NTMR.h"
#include "stdlib.h"
#include "ioCC2530.h"
#include "nwdebuger.h"


/**
@file 
@brief Модуль управления таймером сна
@details При создании обьекта с помощью NT_Create, производится иницилизация
 аппаратного таймера сна и он переходит в активное состояние. 
 Аппаратный таймер сделан на 24х битном счетчике, значение которого изменить
 нельзя. В свою очередь время сети имеет значения шириной в 15 бит.
*/


// После вызова EventCallback аппаратное прерывание завершается
// и управление передается в главный цикл main.
// Пользователь сам должен положить систему в сон
// Для работы используется sleep timer.
// Значение таймера изменить нельзя, он автоматически стартует после
// Включения микроконтроллера. Нам нужно выбрать XOSC вместо RCOSC,
// Но на отладочной плате нет резонатора
// Чтение значения ST2:ST1:ST0 первым из ST0
// Таймер работает во всех режимах кроме PM3
// После пробуждения нужно убеиться что SLEEPSTA.CLK32K бит установлен прежде 
// чем считывать ST0-3


// Публичные методы
NT_s* NT_Create(void);
bool NT_Delete(NT_s *nt);

// Методы класса
bool NT_SetTime(uint16_t ticks);
bool NT_SetCapture(uint16_t ticks);
void NT_IRQEnable(bool state);
void NT_SetEventCallback(void (*fn)(uint16_t ticks));
uint16_t NT_GetTime(void);

// Приватные методы
static uint32_t ReadTimer(void);
static void loadTimerCompare(uint32_t ticks);

// Переменные класса
void (*EventCallback)(uint16_t ticks);
static uint32_t COMPARE_TIME; //!< Значение загруженное в регистр compare
static bool FiredFlag; //!< Флаг указывает, что прерывание состоялось 

/**
@details Смещение времени сети относительно времени таймера.
 NETWORK TIME = TIMER + TOFFSET
*/
uint16_t TOFFSET; 

/**
@brief Создание структуры LLC_s
@param[in] mac Указатель на структуру mac
@return Указатель на структуру LLC_s или NULL
*/
NT_s* NT_Create(void)
{
  // Заполняем структуру указателей
  NT_s* nt = (NT_s*)malloc(NT_S_SIZE);
  nt->NT_SetTime = NT_SetTime;
  nt->NT_SetCapture = NT_SetCapture;
  nt->NT_IRQEnable = NT_IRQEnable;
  nt->NT_SetEventCallback = NT_SetEventCallback;
  nt->NT_GetTime = NT_GetTime;
  TOFFSET = 0;
  COMPARE_TIME = 0;
  FiredFlag = false;
  EventCallback = NULL;
  NT_IRQEnable(false);
  return nt;
}

/**
@brief Удаление обьекта
*/
bool NT_Delete(NT_s *nt)
{
  NT_IRQEnable(false);
  free(nt);
  return true;
}

/**
@brief Устанавливаем текущее время сети и переустанавливает capture time
@param[in] ticks время от 0-32767. Один tick 1/32768
@return true если аргумент в диапазоне
*/
bool NT_SetTime(uint16_t ticks)
{
  ASSERT_HALT(ticks < 32768, "Incorrect ticks");
  if (ticks > 32767)
    return false;
 
  uint16_t timer = ReadTimer();
  TOFFSET = ticks - timer;
  TOFFSET &= 0x7FFF;

  // После установки времени нужно изменить compare time в таймере
  // Но только в случаи если прерывание установленно
  if (!FiredFlag)
  {
    NT_SetCapture(COMPARE_TIME);
  }

  return true;
}

/**
@brief Устанавливает время прерывания
@details Время прерывания устанавливается относительно времени сети
,к примеру, что бы проснуться в таймслот номер 0, нужно установить ticks = 0.
Процедура учитывает текущее значение таймера и перерасчитывает значение ticks. 
@params[in] ticks время сети в тиках когда нужно проснуться
*/
bool NT_SetCapture(uint16_t ticks)
{
  ASSERT_HALT(ticks < 32768, "Incorrect ticks");
  
  uint32_t timer = ReadTimer(); // Текущее значение счетчика
  //NETWORK TIME = TIMER + TOFFSET
  uint16_t network_time = (timer + TOFFSET) & 0x7FFF; // Текущее  временя сети
  uint32_t compare_time;
  
  COMPARE_TIME = ticks; // Сохраняем установленное значение
  FiredFlag = false;
  

  
  if (ticks > network_time)
  {
      compare_time =  (timer & (~0x7FFF)) | ticks;
  }
  else
  {
    compare_time =  (timer & (~0x7FFF)) | ticks;
    compare_time += 0x8000;
    compare_time &= 0xFFFFFF;
  }
 
  loadTimerCompare(compare_time);
  return true;
}

/**
@brief Разрешение прерываний таймера сна
@params[in] state = true - разрешить обработку прерываний
*/
void NT_IRQEnable(bool state)
{
  STIF = 0;
  if (state)
  {
    STIE = 1;
  }
  else
  {
    STIE = 0;
  }
}

/**
@brief Устанавливает обработчик прерывания таймера
@param[in] fn(uint16_t ticks) указатель на функцию-обработчик
*/
void NT_SetEventCallback(void (*fn)(uint16_t ticks))
{
  EventCallback = fn;
}

/**
@brief Возвращает время сети 
@return Время сети в тиках
*/
uint16_t NT_GetTime(void)
{
  uint32_t val = ReadTimer();
  // TOFFSET = NETWORK TIME - TIMER
  // NETWORK TIME = TIMER + TOFFSET
  val +=TOFFSET;
  val &= 0x7FFF; // Приводим значения таймера к 0-32767
  return val;
}

/**
@brief Обработчик прерывания таймера сна
*/
#pragma vector=ST_VECTOR
__interrupt void TimerCompareInterrupt(void)
{
  // Установленное время прерывание уже было обслуженно, а новое не
  // установленно
  if (FiredFlag)
    return;
  
  uint16_t ticks = NT_GetTime();
  if (EventCallback == NULL)
    return;
  EventCallback(ticks); // Вызываем пользовательский обработчик
  STIF = 0; // Очищаем флаг прерывания
  FiredFlag = true;
}

/**
@brief Возвращает текущее значение таймера
@return Текущие ticks
*/
static uint32_t ReadTimer(void)
{
  union 
  {
    uint32_t val;
    uint8_t fild[4];
  } ret_val;
  
  ret_val.val = 0;
  
  // Ждем синхронизацию таймера после пробуждения
  while (!(SLEEPSTA & 0x01));
  
  // Соблюдаем порядок чтения регисторов ST
  ret_val.fild[0] = ST0;
  ret_val.fild[1] = ST1;
  ret_val.fild[2] = ST2;
  return ret_val.val;
}

/**
@brief Устанавливает время пробуждения микроконтролера
@params[in] ticks 24 битное значение времени пробуждения в ticks
*/
static void loadTimerCompare(uint32_t ticks)
{
  union 
  {
    uint32_t val;
    uint8_t fild[4];
  } value;
  
  value.val = ticks;
  
  // Ждем разрешения на запись нового значения
  while (!(STLOAD & 0x01));
  
  // Регистр ST0 должен быть записан в последнию очередь
  ST2 = value.fild[2];
  ST1 = value.fild[1];
  ST0 = value.fild[0];
}

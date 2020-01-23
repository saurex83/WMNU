#include "NTMR.h"
#include "stdlib.h"
#include "ioCC2530.h"
#include "nwdebuger.h"

/**
@file 
@brief Модуль управления таймером сна
@details При создании обьекта с помощью NT_Create, производится иницилизация
 аппаратного таймера сна и он переходит в активное состояние. Если установлено 
 определение #define USE_OSC32K будет использван внешний кварц.
 За установку глобального прерывания IE отвечает пользователь.
 Для использования модуля необходимо установить обработчик прерывания
 NT_SetEventCallback(..). Далее с помощью NT_SetCompare(..) установить
 время срабатывания прерывания, она же включит прерывание таймера. 
 Перед вызовом пользовательского обработчика, запрещается прерывание таймера,
 пользователь должен установить его повторно на необходимое время.
 Модуль сна пробуждает МК из режимов PM1,PM2, но не переводит их обратно.
 После завершения обработки прерывание, продолжится выполнение основного 
 потока программы. Пользователь самостоятельно должен перевести МК в режим сна.
 Функция NT_SetTime(..) производит подстройку времени сети и корректирует время
 срабатывания прерывания. Что бы было понятнее приведу пример: 
 Время сети 400 и установленно прерывание на 700.
 Пользователь корректирует время на 300, но из-за разной разрядности счетчиков
 прерывание произойдет по "старым часам", так как NT_SetTime только изменит
 переменную относительного смещения, а не непосредственно значение счетчика.
 Поэтому необходимо пересчитать время прерывания 700 к новому смещению.
 Из найденных особенностей: когда вызывается обработчик прерывания, значение
 времени всегда больше на 2 тика. Предполагаю из за ожидания синхронизации при
 чтении значения таймера. 
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


#define USE_OSC32K // Использовать внешний квац 32.768 кГц

// Методы модуля
bool NT_SetTime(uint16_t ticks);
void NT_SetCompare(uint16_t ticks);
void NT_SetEventCallback(void (*fn)(uint16_t ticks));
uint16_t NT_GetTime(void);
uint16_t NT_WaitTime(uint16_t ticks);

// Приватные методы
static uint32_t ReadTimer(void);
static void loadTimerCompare(uint32_t ticks);
static inline bool isIRQEnable(void);
static inline void NT_IRQEnable(bool state);

// Переменные модуля
static void (*EventCallback)(uint16_t ticks);
static uint32_t COMPARE_TIME; //!< Значение загруженное в регистр compare

/**
@details Смещение времени сети относительно времени таймера.
 NETWORK TIME = TIMER + TOFFSET
*/
uint16_t TOFFSET; 

/**
@brief Инизиализация таймера
*/
void NT_Init(void)
{
  TOFFSET = 0;
  COMPARE_TIME = 0;
  EventCallback = NULL;

  // Сначало нужно настроить кварц 32кГц только потом поднимать
  // основной квар до 32 Мгц
  #ifdef USE_OSC32K
  CLKCONCMD &= ~(1<<7); // Сбрасываем бит OSC32K, будет выбран 32.768 кварц
  while (CLKCONSTA & (1<<7)); // Ждем пока бит не станет 0
  #endif

  // Переходим на 32 Мгц
  CLKCONCMD = 0;// (1<<3);
  while (CLKCONSTA&(1<<6));
  
  NT_IRQEnable(false);
  EA = 1;
}

/**
@brief Устанавливаем текущее время сети и переустанавливает capture time
@param[in] ticks время от 0-32767. Один tick 1/32768
@return true если аргумент в диапазоне
*/
bool NT_SetTime(uint16_t ticks)
{
  ASSERT(ticks < 32768);
  if (ticks > 32767)
    return false;
 
  uint16_t timer = ReadTimer();
  TOFFSET = ticks - timer;
  TOFFSET &= 0x7FFF;

  // После установки времени нужно изменить compare time в таймере
  // Но только в случаи если прерывание активно
  if (isIRQEnable())
  {
    NT_SetCompare(COMPARE_TIME);
  }

  return true;
}

/**
@brief Вычисляет значение, которое нужно установить в регистр compare таймера.
@params[in] ticks время сети в тикак
*/
static inline uint32_t calcCompareTime(uint16_t ticks)
{   
    uint32_t timer = ReadTimer(); // Текущее значение счетчика
    
    //NETWORK TIME = TIMER + TOFFSET  
    // Приводим такты к тактам таймера
    uint16_t ticks_offset = (ticks - TOFFSET) & 0x7FFF;
    
    uint32_t cmp_time = timer & ~0x7FFF; // Убираем младшие 15 бит
    cmp_time |= ticks_offset; // Вычисляем новое время
    
    if (cmp_time <= timer)
    {
      cmp_time += 0x8000;
      cmp_time &=0xFFFFFF;
    }
      
     LOG_OFF("Timer = %lu, Ticks = %d,CMP = %lu \r\n",
          timer, ticks, cmp_time );
    return cmp_time;
}

/**
@brief Устанавливает время прерывания
@details Время прерывания устанавливается относительно времени сети
,к примеру, что бы проснуться в таймслот номер 0, нужно установить ticks = 0.
Процедура учитывает текущее значение таймера и перерасчитывает значение ticks. 
@params[in] ticks время сети в тиках когда нужно проснуться
*/
void NT_SetCompare(uint16_t ticks)
{
  ASSERT(ticks < 32768);
  
  COMPARE_TIME = ticks; // Сохраняем установленное значение
 
  uint32_t compare_time = calcCompareTime(ticks);
 
  loadTimerCompare(compare_time);
  NT_IRQEnable(true);
}

/**
@brief Разрешение прерываний таймера сна
@params[in] state = true - разрешить обработку прерываний
*/
static inline void NT_IRQEnable(bool state)
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
@brief Проверка активности прерывания таймера
@return true если прерывание установленно
*/
static inline bool isIRQEnable(void)
{
  if (STIE)
    return true;
  else 
    return false;
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
  uint16_t ticks = NT_GetTime();
  if (EventCallback == NULL)
    return;
  // Отключаем прерывание таймера. Забота пользователя его включить
  NT_IRQEnable(false); 
  EventCallback(ticks); // Вызываем пользовательский обработчик
  STIF = 0; // Очищаем флаг прерывания
}

/**
@breif Ожидание наступления времени сети
@param[in] ticks время в тактах
@return фактическое время сети
*/
uint16_t NT_WaitTime(uint16_t ticks)
{
  static union 
  {
    uint32_t val;
    uint8_t fild[4];
  } val;
  
  val.val = 0;
  // TOFFSET = NETWORK TIME - TIMER
  // NETWORK TIME = TIMER + TOFFSET
  
  // Ждем синхронизацию таймера после пробуждения
  while (!(SLEEPSTA & 0x01));
  
  do
  {
    // Соблюдаем порядок чтения регисторов ST  
    val.fild[0] = ST0;
    val.fild[1] = ST1;
    val.fild[2] = ST2;
    val.val +=TOFFSET;
    val.val &= 0x7FFF; // Приводим значения таймера к 0-32767    
  } while (val.val < ticks);
  return val.val;
}

/**
@brief Возвращает текущее значение таймера
@return Текущие ticks
*/
static uint32_t ReadTimer(void)
{
  static union 
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

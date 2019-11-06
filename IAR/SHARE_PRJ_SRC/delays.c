/*!
\file 
\brief Функции измерения времени и задержки
*/

#include "ioCC2530.h"
#include "stdint.h"

#define KHZ_250 0x38 // Делитель до 250кгц
#define DIV_128 0xE0 // Прескалер на 128
#define TIM_START 0x10 // Запуск таймера
#define TICK_US (uint32_t)512 // Один такт в микросекундах (1/250кГц)*128
#define TIMER_MAX (uint32_t)255 // Максимальное значение счетчика
#define TIME_MAX (TICK_US*TIMER_MAX) // Максимальное время
#define TICK_NS  32.25 // Количество нс в одном такте TIM2
#define TIM2_MAX 2047968.75 // 65535 * TICK_NS

// Публичные методы
void TIM_init(void);
uint32_t TIM_getTime();
uint32_t TIM_passedTime(uint32_t time);
void TIM_delayUs(uint32_t delay);

// Таймер MAC
uint16_t TIM_getMACTicks();
static uint16_t getMACTimeDelta(uint16_t start, uint16_t stop);
uint32_t TIM_MAC_NS(uint16_t start, uint16_t stop);

static uint16_t TIM2_CALIB;

/**
@brief Иницилизация таймера
*/
void TIM_init(void)
{
  CLKCONCMD |= KHZ_250; 
  T4CTL |= DIV_128 | TIM_START;
  T2CTRL = 0x01; // ЗАпускаем MAC таймер без синхронизации с осцилятором
  
  uint16_t start = TIM_getMACTicks();
  uint16_t stop  = TIM_getMACTicks();
  TIM2_CALIB = getMACTimeDelta(start, stop);
}

/**
@brief Возвращает время в мкс. Максимальное значение 130560 мкс 
*/
uint32_t TIM_getTimeUs()
{
  return T4CNT * TICK_US;
}

/**
@brief Возвращает такты таймера. 
*/
uint16_t TIM_getMACTicks()
{
  uint16_t low = T2M0;
  uint16_t high = T2M1 << 8;
  uint16_t val = 0;
  val = low;
  val |= high;
  return val;
}

/**
@brief Возвращает количество прошедших тактов таймера между start и stop.
*/
static uint16_t getMACTimeDelta(uint16_t start, uint16_t stop)
{
  uint32_t passed;
 
  if (stop >= start)
    passed = stop - start;
  else
    passed = start - stop;
  
  passed -= TIM2_CALIB;
  return passed;
}

/**
@brief Возвращает прошедшее время в нс. Максимальное значение 2047937 нс
@param[in] time отметка времени в мкс. Не более 2047968 нс
@return Время в нс прошедшее с момента time
*/
uint32_t TIM_MAC_NS(uint16_t start, uint16_t stop)
{
  uint32_t passed = getMACTimeDelta(start, stop);
  passed = (uint32_t)((float)passed * (float)TICK_NS);
  return passed;
}

/**
@brief Возвращает прошедшее время в мкс. Максимальное значение 130048 мкс
@param[in] time отметка времени в мкс. Не более 130560 мкс
@return Время в мкс прошедшее с момента time
*/
uint32_t TIM_passedTimeUs(uint32_t time)
{
  uint32_t now = TIM_getTimeUs();
  if (now >= time)
    return (now-time);
  else
    return (TIME_MAX - time) + now;
}

/**
@brief Временая задержка не более чем на 130048 мкс
*/
void TIM_delayUs(uint32_t delay)
{
  uint32_t start = TIM_getTimeUs();
  while ( TIM_passedTimeUs(start) < delay);
}
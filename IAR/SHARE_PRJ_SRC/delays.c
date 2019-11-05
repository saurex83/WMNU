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

// Публичные методы
void TIM_init(void);
uint32_t TIM_getTime();
uint32_t TIM_passedTime(uint32_t time);
void TIM_delayUs(uint32_t delay);

/**
@brief Иницилизация таймера
*/
void TIM_init(void)
{
  CLKCONCMD |= KHZ_250; 
  T4CTL |= DIV_128 | TIM_START;
}

/**
@brief Возвращает время в мкс. Максимальное значение 130560 мкс 
*/
uint32_t TIM_getTimeUs()
{
  return T4CNT * TICK_US;
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
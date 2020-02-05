/*!
\file  Модуль измерения интервалов времени и функция задержки времени
\brief Функции измерения времени и задержки
*/

#include "ioCC2530.h"
#include "delays.h"
#include "string.h"

#define TICK_NS  31.25 // Количество нс в одном такте TIM2
#define TICK_US  0.03125 // Количество мкс в одном такте TIM2
#define TICK_PER_US 32 // Количество тактов в одной микросекунде

// Максимальное кол-во тактов таймера. Константа 40 бит, 5 байт
static const uint32_t MAC_TIMER_MAX = 0xFFFFFFFFUL; 
static uint32_t DELAY_CALIBRATE_TICS; // Калибровка функции задержки

// Публичные методы
void TIM_init(void);
void TIM_TimeStamp(TimeStamp_s* timestamp);
void TIM_delay(uint32_t delay);
uint32_t TIM_passedTime(TimeStamp_s* start, TimeStamp_s* stop);
void TIM_copy(TimeStamp_s *src, TimeStamp_s *dst);

// Приватные
static inline uint32_t Interval(TimeStamp_s* start, TimeStamp_s* stop);
static void DelayCalibrate(void);

/**
@brief Иницилизация таймера
@brief Используется таймер 2. MAC таймер
*/
void TIM_init(void)
{
  // Запускаем таймер, LATCH MODE = 1
  // Latch mode фиксирует значение таймера переполнения при чтении T2M0
  // ЗАпускаем MAC таймер без синхронизации с кварцем 32.768к
  T2CTRL = (1<<0) | (1<<3); 
  // Калибровка функции задержки
  DelayCalibrate();
}

/**
@brief Возвращает временую метку.
@params[out] timestamp указатель на временную метку 
*/
void TIM_TimeStamp(TimeStamp_s* timestamp)
{
  timestamp->byte[0] = T2M0;
  timestamp->byte[1] = T2M1;
  timestamp->byte[2] = T2MOVF0;
  timestamp->byte[3] = T2MOVF1;
}

/**
@brief Калибровка функции задержки TIM_delay
*/
static void DelayCalibrate(void)
{
  #define ACCURATE_VAL_US 500 
  DELAY_CALIBRATE_TICS = 0;
  TimeStamp_s start, stop;
  uint32_t passed, cal;
  
  TIM_TimeStamp(&start);
  TIM_delay(ACCURATE_VAL_US);
  TIM_TimeStamp(&stop);
  passed = TIM_passedTime(&start, &stop);
  
  cal = passed - ACCURATE_VAL_US;
  DELAY_CALIBRATE_TICS = cal * TICK_PER_US; 
};

/**
@brief Вычисляет количество тактов между двумя времеными метками
@return Количество тактов
*/
static inline uint32_t Interval(TimeStamp_s* start, TimeStamp_s* stop)
{
  uint32_t ret;

  if (stop->timer >= start->timer)
    ret = stop->timer - start->timer;
  else
  { 
    ret = (MAC_TIMER_MAX - start->timer) + stop->timer;
  }
  return ret;
}

/**
@brief Вычисляет прошедшее время между двумя времеными метками
@return Время в мкс.
*/
uint32_t TIM_passedTime(TimeStamp_s* start, TimeStamp_s* stop)
{
  uint32_t ret = Interval(start, stop);
  
  ret = (uint32_t)((float)ret * TICK_US);
  return ret;
}

/**
@brief Задержка на заданное количество мкс. 
@details Погрешность около 20 мкс при значениях выше 150 мкс.
 Рекомендуется использовать для организации задержек не менее 200 мкс.
@params[in] delay количество микросекунд. Не более 134217727 мкс (134.2 сек)
*/
void TIM_delay(uint32_t delay)
{
  uint32_t passed = 0;
  TimeStamp_s start, now;
  TIM_TimeStamp(&start);
  
  // Конвертируем мкс в такты и вносим поправку
  delay = delay * TICK_PER_US; 
  if (delay > DELAY_CALIBRATE_TICS)
    delay -= DELAY_CALIBRATE_TICS;
  
  do
  {
    TIM_TimeStamp(&now);
    passed = Interval(&start, &now);
  } while (passed < delay);
}

/**
@brief Копирование временной метки
@param[in] src указатель на исходную метку для копирование
@param[out] dst указатель на метку назначения
*/
void TIM_copy(TimeStamp_s *src, TimeStamp_s *dst)
{
  memcpy(src, dst, sizeof(TimeStamp_s));
}
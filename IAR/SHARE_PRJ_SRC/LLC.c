/**
@file 
@brief Модуль содержит реализацию интерфейса радио для приема/передачи данных.
*/

#include "LLC.h"
#include "MAC.h"
#include "TIC.h"
#include "stdlib.h"
#include "nwdebuger.h"

// Публичные методы
void LLC_Init(void);

// Методы модуля
void LLC_SetRXCallback(void (*fn)(frame_s *fr));
void LLC_TimeAlloc(void (*fn)(void)); 
void LLC_AddTask(frame_s* fr);

// Закрытые методы
static void LLC_SE_HNDL(uint8_t TS); //!< Обработчик завешения слота TIC
static void LLC_Shelduler(uint8_t TS);
static void LLC_RX_HNDL(frame_s *fr); //!< Обработчик примема пакета
static void LLC_RunTimeAlloc(void);

// Переменные модуля
#define SEND_ATEMPTS 5 //!< Количество попыток передачи пакета 

typedef struct LLCTask LLCTask;
typedef struct TimeAllocFunc TimeAllocFunc;

/**
@brief Описание задачи модуля LLC
*/
struct LLCTask
{
  LLCTask *next; //!< Указатель на следующую задачу. NULL - конец очереди
  uint8_t TS; //!< Номер временого канала для передачи сообщения
  uint8_t CH; //!< Номера радиоканала для передачи сообщения
  frame_s *fr; //!< Указатель на данные для передачи
};

/**
@brief Список функций вызываемых по завешению временного слота
*/
struct TimeAllocFunc
{
  TimeAllocFunc *next; //!< Указатель на следующую функцию. NULL - конец списка
  void (*fn)(void); //!< Указатель на функцию. NULL - нет обработчика
};

/**
@brief Первый элемент списка задач.
@details Элемент создается статически, все остальные задачи создаются malloc
и добавляются к HeadTask.
*/
static LLCTask HeadTask;

/**
@brief Обратный вызов при приеме пакета данных
*/
static void (*RXCallback)(frame_s *fr);

/**
@brief Первый элемент обработчика конца временного слота
@details Элемент создается статически, все остальные задачи создаются malloc
и добавляются к HeadAllocFunc.
*/
static TimeAllocFunc HeadAllocFunc;

/**
@brief Инициализация модуля
*/
void LLC_Init(void)
{  
  // Регистрируем обработчики
  TIC_SetSECallback(LLC_SE_HNDL); // Завершение временного слота
  MAC_SetRXCallback(LLC_RX_HNDL); // Принято сообщение
}


void LLC_SetRXCallback(void (*fn)(frame_s *fr))
{
  ASSERT_HALT(fn != NULL, "NULL pointer not allow");
  RXCallback = fn;
}

/**
@brief Добавляет обработчик заверешния временого слота в список
*/
void LLC_TimeAlloc(void (*fn)(void))
{
  TimeAllocFunc *ta = malloc(sizeof(TimeAllocFunc));
  ASSERT_HALT(ta != NULL, "Memory allocation fails");
  ta->next = NULL;
  ta->fn = fn;

  // Перебираемся в конец списка
  TimeAllocFunc *next = &HeadAllocFunc;
  while (next->next != NULL)
    next = next->next;
  
  // Добавляем новый элемент
  next->next = ta;
};

/**
@brief Добавляем задачу в очередь
*/
void LLC_AddTask(frame_s* fr)
{
}

/**
@brief Планировщик задач
@details Соотносит задачи в очереди и свободные временные слоты MAC уровня.
При освобождении временных слотов, планировщик устанавливает данные для 
передачи в MAC.
@param[in] TS номер текущего временого слота
*/
static void LLC_Shelduler(uint8_t TS)
{
  // Перебираем попорядку весь список на отправку
  // HeadTask только указатель на первый элемент, сам он не подлежит отправке
  LLCTask *task = &HeadTask;
  LLCTask *last = NULL;
  
  while (task->next != NULL)
  {
    last = task;
    task = task->next;
    if (MAC_GetTXState(task->TS)) // Занят ли временой слот
      continue; // Если слот занят переходим к следующему
    
    MAC_Send(task->fr, SEND_ATEMPTS);
    
    // Удаляем из списка
    last->next = task->next;
    free(task);
  }
}

/**
@brief Обработчик заверешния временого слота
@details Обратный вызов из TIC. Обработчик вызывает планировщик задач очереди
и передает управление зарегистрированым функциям с помощью LLC_TimeAlloc.
@param[in] TS номер завершенного временого слота
*/
static void LLC_SE_HNDL(uint8_t TS)
{
  LLC_Shelduler(TS);
  LLC_RunTimeAlloc();
}

static void LLC_RX_HNDL(frame_s *fr)
{
  if (RXCallback != NULL)
    RXCallback(fr);
}

/**
@brief Передаем управление зарегистрированным функциям
*/
static void LLC_RunTimeAlloc(void)
{
    TimeAllocFunc *next = &HeadAllocFunc;
  // HeadAllocFunc служит только для указания на первый элемент в очереди
  // так что его нужно пропустить и переходить сразу к next
  while (next->next != NULL)
  {
    next->next->fn();
    next = next->next;
  }
}

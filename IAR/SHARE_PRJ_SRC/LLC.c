/**
@file 
@brief Модуль содержит реализацию интерфейса радио для приема/передачи данных.
*/

#include "LLC.h"
#include "MAC.h"
#include "TIC.h"
#include "mem.h"
#include "nwdebuger.h"
#include "ioCC2530.h"

// Публичные методы
void LLC_Init(void);

// Методы модуля
void LLC_SetRXCallback(void (*fn)(frame_s *fr));
void LLC_TimeAlloc(void (*fn)(void)); 
bool LLC_AddTask(frame_s* fr);
uint8_t LLC_GetTaskLen(void);
void LLC_Reset(void);

// Закрытые методы
static void LLC_SE_HNDL(uint8_t TS); //!< Обработчик завешения слота TIC
static void LLC_Shelduler(uint8_t TS);
static void LLC_RX_HNDL(frame_s *fr); //!< Обработчик примема пакета
static void LLC_RunTimeAlloc(void);

// Переменные модуля
#define UNICAST_SEND_ATEMPTS 5 //!< Количество попыток передачи пакета 
#define BROADCAST_SEND_ATEMPTS 3 //!< Количество попыток передачи пакета 
#define MAX_nbrTASKS 20 //!< Максимальное количество задач в очередях

typedef struct LLCTask LLCTask;
typedef struct TimeAllocFunc TimeAllocFunc;
static bool tasksBLOCK; //!< блокировка доступа к LLCTask

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

static LLCTask *FirstTask; //!< Указатель на первый элемент очереди задач
static uint8_t nbrTasks; // Количество задач в очереди

/**
@brief Обратный вызов при приеме пакета данных
*/
static void (*RXCallback)(frame_s *fr);

/**
@brief Первый элемент обработчика конца временного слота
@details Элемент создается статически, все остальные задачи создаются re_malloc
и добавляются к HeadAllocFunc.
*/
static TimeAllocFunc HeadAllocFunc;

/**
@brief Инициализация модуля
@detail Иницилизирует MAC
*/
void LLC_Init(void)
{  
  nbrTasks = 0; 
  tasksBLOCK = false;
  FirstTask = NULL;
  // TODO Очистить очередь HeadTask
  // Регистрируем обработчики
  TIC_SetSECallback(LLC_SE_HNDL); // Завершение временного слота
  MAC_SetRXCallback(LLC_RX_HNDL); // Принято сообщение
}

/**
@brief Сброс модуля
@detail Удаяляем все пакеты в очереди. Аллокатор времени не трогаем
*/
void LLC_Reset(void)
{
  LLCTask *task = FirstTask;
  LLCTask *next;
  
  while (task != NULL)
  {
    next = task;
    re_free(task);
    task = next;
  }
}

/**
@brief Количество задач в очереди
@return количество задач в очереди
*/
uint8_t LLC_GetTaskLen(void)
{
  return nbrTasks;
}

void LLC_SetRXCallback(void (*fn)(frame_s *fr))
{
  ASSERT(fn != NULL);
  RXCallback = fn;
}

/**
@brief Добавляет обработчик заверешения временого слота в список
*/
void LLC_TimeAlloc(void (*fn)(void))
{
  TimeAllocFunc *ta = re_malloc(sizeof(TimeAllocFunc));
  ASSERT(ta != NULL);
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
@brief Взовращает true если задача добавлена в очередь. false, если размер 
 очереди достиг максимального размера.
*/
bool LLC_AddTask(frame_s* fr)
{
   ASSERT(fr != NULL);
 
    if (nbrTasks == MAX_nbrTASKS)
      return false;
    
   // Ждем пока разблокируется доступ.
   while (tasksBLOCK);
   tasksBLOCK = true;

   // Создаем новую задачу
   LLCTask *new_task = (LLCTask*)re_malloc(sizeof(LLCTask));
   ASSERT(new_task !=NULL); 
      
   new_task->TS = fr->meta.TS;
   new_task->CH = fr->meta.CH;
   new_task->fr = fr;
   
  LOG_ON("Add task = %u, CH = %d, TS = %d, OTS = %d. fr = %d",
      (uint16_t)new_task,new_task->CH, new_task->TS, fr->meta.TS, (uint16_t)fr);
  
   // Если в очереди нет задач, добавим первую
   if (FirstTask == NULL) 
   {
     new_task->next = NULL;
     FirstTask = new_task;
   }
   // Если в очереди были задачи то вставим новую в голову списка
   else 
   {
     new_task->next = FirstTask;
     FirstTask = new_task;  
   }
   
   nbrTasks ++;
   tasksBLOCK = false;

   return true;
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
  // Если работает функция добавления задачи, то обслуживание очереди не
  // производится. Это может привести к необычным последствиям
  if (tasksBLOCK)
    return;
  tasksBLOCK = true;
  
  // Перебираем попорядку весь список на отправку
  // HeadTask только указатель на первый элемент, сам он не подлежит отправке
  LLCTask *task = FirstTask;
  LLCTask *last = FirstTask;
  LLCTask *next = FirstTask;
  
  while (task != NULL)
  {    
    if (MAC_GetTXState(task->TS)) // Занят ли временой слот
    {
      // Если слот занят переходим к следующей задаче
      last = task;
      task = task->next;
      continue; 
    }
    
    //Выбераем количество попыток передачи в зависимости от типа пакета
    uint8_t attempts;
    if (task->fr->meta.TX_METHOD == UNICAST)
      attempts = UNICAST_SEND_ATEMPTS;
    else if (task->fr->meta.TX_METHOD == BROADCAST)
      attempts = BROADCAST_SEND_ATEMPTS;
    else
      ASSERT(false);
    
    MAC_Send(task->fr, attempts);
    
    next = task->next; // Запомним следующую задачу
    
    // Удаляем текущую задачу из списка
    if (task == FirstTask)
    {
    // Удаляемым элементом является указателем на голову
       FirstTask = next;
       last = FirstTask;
    }
    else
    {
      // Удаляемым элементом является промежуточный элемент
      last->next = next;
    }
    
    nbrTasks--;
    LOG_OFF("Free task = %u, nbrTasks = %d\r\n",
        (uint16_t)task, nbrTasks); 
    re_free(task);
    task = next;
  }
  
  tasksBLOCK = false;
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
  ASSERT(RXCallback !=NULL);
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

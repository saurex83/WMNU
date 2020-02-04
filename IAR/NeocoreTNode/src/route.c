#include "nwdebuger.h"
#include "ethernet.h"
#include "config.h"
#include "balancer.h"
#include "frame.h"
#include "LLC.h"
#include "mem.h"
#include "neighnbor.h"
#include "string.h"
#include "TIC.h"

/**
@file
@detail
*/

// Определения
#define PID_BIND_TABLE_SIZE 10 //!< Максимальное кол-в протоколов PID
#define ROUTE_RECORDS_S_SIZE sizeof(route_record_s)

// Локальные функции
static void RP_Receive_HNDL(frame_s *fr);
static void RP_TimeAlloc_HNDL(void);
static int find_free_index(void);
static int find_record(uint16_t nsrc, uint16_t fsrc);
static void update_record_time(uint8_t index);
static void insert_record(uint8_t index, uint16_t nsrc, uint16_t fsrc);
static void add_route_record(uint16_t nsrc, uint16_t fsrc);
static bool find_nsrc_by_fdst(uint16_t fdst, uint16_t *nsrc, uint8_t *index);

// Глобальные функции
void RP_Init(void);
void RP_Reset(void);
void RP_Send_GW(frame_s *fr);
void RP_Send(frame_s *fr);
void RP_SendRT_GW(frame_s *fr);
void RP_SendRT_RT(frame_s *fr);
void RP_Bind_PID(uint8_t pid, void (*fn)(frame_s *fr));

// Локальные переменные
typedef struct{
    void (*fn)(frame_s *fr);
    uint8_t pid;
} pid_bind_s;

typedef struct{
  uint16_t nsrc;
  uint16_t fsrc;
  uint32_t update_time;
  bool record_active;
} __attribute__((packed)) route_record_s;

static route_record_s ROUTE_TABLE[ROUTE_TABLE_ITEMS];

static pid_bind_s PID_BIND_TABLE[PID_BIND_TABLE_SIZE];

void RP_Init(void){
  LLC_TimeAlloc(RP_TimeAlloc_HNDL);
  ETH_SetRXCallback(RP_Receive_HNDL);
  RP_Reset();
}

void RP_Reset(void){
  // Таблицу связей не уничтожать
//  for (uint8_t i = 0; i < PID_BIND_TABLE_SIZE; i++){
//    PID_BIND_TABLE[i].fn == NULL;
//    PID_BIND_TABLE[i].pid = 0;
//  }     
  for (uint8_t i = 0; i < ROUTE_TABLE_ITEMS; i++)
       memset(&ROUTE_TABLE[i], 0, ROUTE_RECORDS_S_SIZE);
}

/**
@brief Отправка пакета на шлюз от текущего узла
*/
void RP_Send_GW(frame_s *fr){
  int etx = NP_GetETX(); // Некому передавать пакет
  if (etx == -1){
    frame_delete(fr);
    return;
  }
  
  uint16_t addr;
  uint8_t ts, ch;
  // Извлекаем информацию об соседе через который передаем инфу шлюзу
  if (!NP_Get_COMM_node_info(&addr, &ts, &ch)){
    frame_delete(fr);
    return;
  }
  
  fr->meta.TX_METHOD = UNICAST;
  fr->meta.NSRC = CONFIG.node_adr;
  fr->meta.NDST = addr;
  fr->meta.TS = ts;
  fr->meta.CH = ch;
  ETH_Send(fr);
}

/**
@brief Отправка без модификации данных, как есть
*/
void RP_Send(frame_s *fr){
  ETH_Send(fr);
}

/**
@brief Поиск свободного места в таблице маршрутизации
@return -1 если нет места
*/
static int find_free_index(void){
  for (uint8_t i = 0; i < ROUTE_TABLE_ITEMS; i++)
    if (!ROUTE_TABLE[i].record_active)
      return i;
  return -1;
}

/**
@brief Ищет запись в таблице
@return -1 если нет места
*/
static int find_record(uint16_t nsrc, uint16_t fsrc){
  for (uint8_t i = 0; i < ROUTE_TABLE_ITEMS; i++)
    if (ROUTE_TABLE[i].record_active)
      if (ROUTE_TABLE[i].nsrc == nsrc && ROUTE_TABLE[i].fsrc == fsrc)
        return i;
  return -1;
}


/**
@brief Обновление времения
*/
static void update_record_time(uint8_t index){
  ROUTE_TABLE[index].update_time = TIC_GetUptime();
}

/**
@brief Вставить запись по индексу
*/
static void insert_record(uint8_t index, uint16_t nsrc, uint16_t fsrc){
  ROUTE_TABLE[index].nsrc = nsrc;
  ROUTE_TABLE[index].fsrc = fsrc;
  ROUTE_TABLE[index].record_active = true;
  update_record_time(index);
}

/**
@brief Добавить запись в таблицу маршрутизации или обновить существующую
*/
static void add_route_record(uint16_t nsrc, uint16_t fsrc){
  // Ищем существующую запись
  int record_index = find_record(nsrc, fsrc);
  if (record_index != -1){
    update_record_time(record_index);
    return;
  }
  
  int free_index = find_free_index();
  if (free_index != -1)
    insert_record(free_index, nsrc, fsrc);
}

/**
@brief Отправка пакета с маршрутизацией к шлюзу от другого узла.
 Процесс ретрансляции чущих данных. 
@detail При маршрутизации пакета, нужно сохранить информацию об отпраителе
*/
void RP_SendRT_GW(frame_s *fr){
  uint16_t nsrc = fr->meta.NSRC; // Кто нам передал пакет
  uint16_t fsrc = fr->meta.FSRC; // Узел создавший пакет
  add_route_record(nsrc, fsrc);
  RP_Send_GW(fr);  
}

/**
@brief Поиск промежуточного узла по адресу конечного получателя
*/
static bool find_nsrc_by_fdst(uint16_t fdst, uint16_t *nsrc, uint8_t *index){
  for (uint8_t i = 0; i < ROUTE_TABLE_ITEMS; i++)
    if (ROUTE_TABLE[i].record_active)
      if (ROUTE_TABLE[i].fsrc == fdst){
        *nsrc = ROUTE_TABLE[i].nsrc;
        *index = i;
        return true;
      }
  return false;
}

/**
@brief Отправка пакета с маршрутизацией от шлюза к другому узлу.
 Шлюз кому то передает данные, мы используюя таблицу маршрутизации
 перепосылаем пакет.
*/
void RP_SendRT_RT(frame_s *fr){
  uint16_t fdst = fr->meta.FDST;
  uint16_t nsrc;
  uint8_t index;
  
  if (!find_nsrc_by_fdst(fdst, &nsrc, &index)){ // Не нашли пути
    frame_delete(fr);
    return;
  }

  uint8_t ts,ch;
  if (!NP_Get_info_by_addr(nsrc, &ts, &ch)){ // Промежуточный узел нам не сосед
    frame_delete(fr);
    return;
  }
  
  fr->meta.TX_METHOD = UNICAST;
  fr->meta.NSRC = CONFIG.node_adr;
  fr->meta.NDST = nsrc;
  fr->meta.TS = ts;
  fr->meta.CH = ch;
  update_record_time(index); 
  ETH_Send(fr);  
}

/**
@brief Регистрация обработчика PID
*/
void RP_Bind_PID(uint8_t pid, void (*fn)(frame_s *fr)){
  for (uint8_t i = 0; i < PID_BIND_TABLE_SIZE; i++){
    if (PID_BIND_TABLE[i].fn == NULL){
      PID_BIND_TABLE[i].fn = fn;
      PID_BIND_TABLE[i].pid = pid;
      return;
    }     
  }
}

/**
@brief Обработчик принятого пакета уровня ethernet
*/
static void RP_Receive_HNDL(frame_s *fr){
 //TODO Обнавление таблиц маршрутизации!
  bool found = false;
  for (uint8_t i = 0; i < PID_BIND_TABLE_SIZE; i++){
    if (PID_BIND_TABLE[i].pid == fr->meta.PID){
      found = true;
      PID_BIND_TABLE[i].fn(fr);
      break;
    }
  }
  
  // Нет обработчика нужного PID
  if (!found){
    frame_delete(fr);
    return;
  }
  // Балансеру достаточно метаданных. Если передавать пакет,
  // то становится вопрос а кто его удалять должен?
  meta_s meta;
  re_memcpy(&meta, &fr->meta, META_S_SIZE);
  TB_Receive_HNDL(&meta);
}

/**
@brief Рабочее время модуля
*/
static void RP_TimeAlloc_HNDL(void){
}
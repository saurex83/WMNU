#include "nwdebuger.h"
#include "LLC.h"
#include "frame.h"
#include "route.h"
#include "basic.h"
#include "string.h"
#include "Net_frames.h"
#include "config.h"
#include "ethernet.h"
#include "mem.h"
#include "TIC.h"
#include "stdlib.h"
#include "sync.h"

/**
@file
@detail
Протокол соседей ведет учет узлов в таблице соседей. Таблица используется 
другими протоколами. Основные действия:
 - Принят запрос соседей -> ответить информацией о себе
 - Принята пакет с информацией о соседе - > занести в таблицу
 - Если давно не получали информации о соседях - > отправить запрос(но не часто)
 - Если давно не передавали информацию о себе -> Передать пакет
 - Функция выдачи информации о лучшем соседе до шлюза
 - В качестве отметок времени для таблици RTC_Uptime
*/

// Определения
#define NB_FRAME_CARD_SIZE sizeof(nb_frame_card_s)
#define NB_FRAME_REQ_SIZE sizeof(nb_frame_req_s)
#define NB_TABLE_SIZE sizeof(nb_table_s)
#define NB_TABLE_ITEMS 5 //!< Максимальное число соседей

enum CMD_REQ{REQ_CMD = 0};

// Глобальные функции
void NP_Reset(void);
void NP_Init(void);
int NP_GetETX(void);
bool NP_Get_info_by_addr(uint16_t addr, uint8_t *ts, uint8_t *ch);
bool NP_Get_COMM_node_info(uint16_t *addr, uint8_t *ts, uint8_t *ch);

// Локальные переменные
typedef struct { // Информационная карточка узла
    uint8_t ts_slots[MAX_OPEN_SLOTS]; // Временые слоты приема пакетов
    uint8_t ch_slots[MAX_OPEN_SLOTS]; // Частоты приема пакетов
    uint8_t ETX; // 0 - шлюз, 1 - одна передача до шлюза
}__attribute__((packed)) nb_frame_card_s;

typedef struct { // Какой то запрос от другого узла
  uint8_t cmd_req; // 0 - запрос информации
}__attribute__((packed)) nb_frame_req_s;

typedef struct{
  uint16_t node_addr; // Адрес узла
  int8_t RSSI;
  int8_t LIQ;
  uint8_t weight; // Качество узла от 0 до 255. 0 - плохо
  nb_frame_card_s card; // Информация об узле
  uint32_t update_time; // Время обновления записи
  bool record_active; // Запись действительна 
} nb_table_s;

static nb_table_s NB_TABLE[NB_TABLE_ITEMS];
static uint32_t NEXT_CARD_SEND_TIME;
static uint32_t LAST_TIME_CARD_RECIEVED;
static uint32_t NEXT_CARD_REQ_TIME;
static nb_table_s *COMM_NODE; //!< Узел через который связываемся с шлюзом 

// Локальные функции
static void NP_Receive_HNDL(frame_s *fr);
static void NP_TimeAlloc_HNDL(void);
static bool frame_filter_card(frame_s *fr);
static bool frame_filter_cmd_req(frame_s *fr);
static void process_card(frame_s *fr);
static void process_cmd_req(frame_s *fr);
static void send_card(void);
static int find_index(frame_s *fr);
static void insert_record(frame_s *fr);
static void update_record(frame_s *fr, uint8_t index);
static int find_free_index();
static void deactivate_unused_records(void);
static void card_send_task(void);
static bool is_nb_table_free(void);
static void communication_node_chose(void);
static uint8_t calc_weight(nb_frame_card_s *card);
static int find_by_addr(uint16_t addr);

void NP_Init(void){
  LLC_TimeAlloc(NP_TimeAlloc_HNDL);
  RP_Bind_PID(PID_NP, NP_Receive_HNDL);
  NP_Reset();
}

void NP_Reset(void){
  for (uint8_t i = 0; i < NB_TABLE_ITEMS; i++)
    memset(&NB_TABLE[i], 0x00, NB_TABLE_SIZE);
  NEXT_CARD_SEND_TIME = 0;
  NEXT_CARD_REQ_TIME =0;
  LAST_TIME_CARD_RECIEVED = 0;
  COMM_NODE = NULL;
  LOG_ON("NP Reset");
}

/**
@brief Получить текущий ETX узла
@return -1 нет ETX
*/
int NP_GetETX(void){
  if (COMM_NODE == NULL)
    return -1;
  
  if (!COMM_NODE->record_active)
    return -1;
  
  return COMM_NODE->card.ETX + 1;
}


static int find_by_addr(uint16_t addr){
  // Попробуем найти пакет по адресу
  for (uint8_t i = 0; i < NB_TABLE_ITEMS; i++)
    if (NB_TABLE[i].node_addr == addr)
      if (NB_TABLE[i].record_active)
        return i;
  return -1;  
}

/**
@brief Получить инфу о соседе по адресу
*/
bool NP_Get_info_by_addr(uint16_t addr, uint8_t *ts, uint8_t *ch){
  int index = find_by_addr(addr);
  if (index == -1)
    return false;

  nb_frame_card_s *card = &NB_TABLE[index].card;
  
  // Считаем что узла всегда открыты как минимум 1 слот
  // Узнаем сколько открыто слотов. Открываются попорядку в массиве.
  uint8_t nbSlots = 0;
  for (uint8_t i = 0; i < MAX_OPEN_SLOTS; i++)
    if (card->ts_slots[i] != 0)
      nbSlots++;
    else
      break;
    
  uint8_t chose = rand() % nbSlots;
  
  ASSERT(nbSlots != 0);
  if (nbSlots == 0) // Вобще то такого не должно быть совсем
    return false;
  
  *ts = card->ts_slots[chose];
  *ch = card->ch_slots[chose];
  return true;  
  
}

/**
@brief Получить таймслот, адрес и канал ближайшего соседа для передачи пакета
 в направлении шлюза
@return -1 нет ETX
*/
bool NP_Get_COMM_node_info(uint16_t *addr, uint8_t *ts, uint8_t *ch){
  if (COMM_NODE == NULL)
    return false;
  
  if (!COMM_NODE->record_active)
    return false; 
  
  *addr = COMM_NODE->node_addr;
  // У узла может быть несколько временых слотов. Выбираем случайным образом
  
  // Считаем что узла всегда открыты как минимум 1 слот
  // Узнаем сколько открыто слотов. Открываются попорядку в массиве.
  uint8_t nbSlots = 0;
  for (uint8_t i = 0; i < MAX_OPEN_SLOTS; i++)
    if (COMM_NODE->card.ts_slots[i] != 0)
      nbSlots++;
    else
      break;
    
  uint8_t chose = rand() % nbSlots;
  
  ASSERT(nbSlots != 0);
  if (nbSlots == 0) // Вобще то такого не должно быть совсем
    return false;
  
  *ts = COMM_NODE->card.ts_slots[chose];
  *ch = COMM_NODE->card.ch_slots[chose];
  return true;
}

static bool frame_filter_cmd_req(frame_s *fr){
    nb_frame_req_s *cmd_req = (nb_frame_req_s*)fr->payload;
  
  // Фильтр 1: по размеру кадра
  if (fr->len < NB_FRAME_REQ_SIZE)
    return false;

  // TODO проверить доступные команды

  return true;

}

static bool frame_filter_card(frame_s *fr){
    nb_frame_card_s *card = (nb_frame_card_s*)fr->payload;
  
  // Фильтр 1: по размеру кадра
  if (fr->len < NB_FRAME_CARD_SIZE)
    return false;

  // TODO Тут интелектуалные фильтры по содержимому
  // Желательно проверять что именно нам прислали и являются
  // ли данные корректными

  return true;
}

/**
@brief Приняли пакет протокола соседей
*/
static void NP_Receive_HNDL(frame_s *fr){
  // пакеты с этого адреса не обрабатываем
  if (fr->meta.NSRC == 0xffff){
    frame_delete(fr);
    return;
  }   
  if (frame_filter_card(fr))
      process_card(fr);
  else if (frame_filter_cmd_req(fr))
      process_cmd_req(fr);
    else 
      frame_delete(fr); // Если под фильтры не подходит пакет
  // Пакет уничтожают функции
}

/**
@brief Ищет запись в масиве соседей об узле приславшем пакет
@return -1 если информации нет
*/
static int find_index(frame_s *fr){
  // Попробуем найти пакет
  for (uint8_t i = 0; i < NB_TABLE_ITEMS; i++)
    if (NB_TABLE[i].node_addr == fr->meta.NSRC)
      if (NB_TABLE[i].record_active)
        return i;
  return -1;
}

/**
@brief Ищем свободное место
@return -1 
*/
static int find_free_index(){
  for (uint8_t i = 0; i < NB_TABLE_ITEMS; i++)
    if (!NB_TABLE[i].record_active)
      return i;
  return -1;
}

/**
@brief Обновление карточки по индексу
*/
static void update_record(frame_s *fr, uint8_t index){
  nb_frame_card_s *card = (nb_frame_card_s*)fr->payload;
  re_memcpy(&NB_TABLE[index].card, card, NB_FRAME_CARD_SIZE);
  NB_TABLE[index].update_time = TIC_GetUptime();
  uint8_t weight = calc_weight(card);
  NB_TABLE[index].weight = weight;
  NB_TABLE[index].node_addr = fr->meta.NSRC;
  NB_TABLE[index].record_active = true;
  NB_TABLE[index].RSSI = fr->meta.RSSI_SIG;
  NB_TABLE[index].LIQ = fr->meta.LIQ;
}

/**
@brief Вычисляет вес карты
@return 0 - плохо, 255 отлично
*/
static uint8_t calc_weight(nb_frame_card_s *card){
  // Сравнивает две карты по ETX, LIQ(пока не доступно), RSSI
  // ПОДУМАТЬ. При маршрутизаии от шлюза сохраняются маршруты до соседей
  // Если нет соседа ( выкинули его) то маршрут разрушится
  return 0;
}

/**
@brief Вставка новой записи в таблицу.
@detail Вставка происходит если в таблице есть место. В противном случаии
проверяется ищится запись хуже по параметрам чем эта и заменяется.
*/
static void insert_record(frame_s *fr){
  int index = find_free_index();
  nb_frame_card_s *card = (nb_frame_card_s*)fr->payload;
  uint8_t weight = calc_weight(card);
  
  if (index != -1){ // Нашли место, запихиваем карточку
    update_record(fr,index);
    LOG_ON("Card inserted.")
    return;
  }
  
  uint8_t bad_index, bad_weight = 255;
  bool found = false;
  
  // Свободных мест нет. Ищем самую плохую запись
  for (uint8_t i = 0; i < NB_TABLE_ITEMS; i++)
    if (NB_TABLE[i].record_active){
      if (NB_TABLE[i].weight < bad_weight){
        bad_weight = NB_TABLE[i].weight ;
        bad_index = i;
        found = true;
       }
    }
  
  if (!found){ // Если нету записей.
    LOG_ON("Card is bad. not inserted")
    return;
  }
  
  if (weight > bad_weight){ //// Карточка лучше чем самая плохая в таблице
    LOG_ON("Better card inserted")
    update_record(fr, bad_index);
  }
}

/**
@brief Обработка принятой карты 
*/
static void process_card(frame_s *fr){
  int index = find_index(fr);
  
  if (index == -1) // Если нет записи об этом узле, вставим
    insert_record(fr);
  else{ // Если запись есть, то обновим 
    LOG_ON("Update card")
    update_record(fr, index);
  }

  // Обновим время получения последней карточки
  // мне не важно вставили или нет, главное что они регулярно приходят
  LAST_TIME_CARD_RECIEVED = TIC_GetUptime();
  frame_delete(fr);
}
   
/**
@brief Обработка принятой команды
*/
static void process_cmd_req(frame_s *fr){
  nb_frame_req_s *cmd_req = (nb_frame_req_s*)fr->payload;  
   
  // Запрос информации об узле
  if (cmd_req->cmd_req == REQ_CMD)
    send_card();
  LOG_ON("CMD reques processed.")
  frame_delete(fr);
}

/**
@brief Создание и отправка карточки узла
*/
static void send_card(void){
  nb_frame_card_s card;
  
  int etx = NP_GetETX();
  // Выходим если ETX не определен
  if (etx == -1){
    // Раз нет ETX то продлим время отправки карты
    uint32_t now = TIC_GetUptime();
    NEXT_CARD_SEND_TIME = now + NEIGHBOR_CARD_SEND_INTERVAL + 
      rand() % NEIGHBOR_CARD_SEND_INTERVAL_DEV;
    LOG_ON("ETX not defind. Card not sended.")
    return;
  }
  
  card.ETX = etx;
  
  for (uint8_t i = 0; i < MAX_OPEN_SLOTS; i++){
  card.ts_slots[i] = CONFIG.ts_slots[i];
  card.ch_slots[i] = CONFIG.ch_slots[i];
  }
  
  frame_s *fr = frame_create();
  frame_addHeader(fr, &card, NB_FRAME_CARD_SIZE);
  fr->meta.PID = PID_NP;
  fr->meta.NDST = 0xffff;
  fr->meta.NSRC = CONFIG.node_adr; 
  fr->meta.TS = 0;
  fr->meta.CH = CONFIG.sys_channel;
  fr->meta.TX_METHOD = BROADCAST;
  RP_Send(fr);
  LOG_ON("NP Card sended");
}

/**
@brief Записи которые давно не обновлялись нужно удалит
*/
static void deactivate_unused_records(void){
  uint32_t alive_time;
  for (uint8_t i = 0; i < NB_TABLE_ITEMS; i++)
    if (NB_TABLE[i].record_active){
      alive_time = TIC_GetUptime() - NB_TABLE[i].update_time;
      if (alive_time > NEIGHBOR_ALIVE_TIME){
        LOG_ON("Record deactivated"); 
        NB_TABLE[i].record_active = false;
      }
    }
}

/**
@brief Производит отправку карточки узла по расписанию
*/
static void card_send_task(void){
  uint32_t now = TIC_GetUptime();
  if (now > NEXT_CARD_SEND_TIME){
    LOG_ON("Time to send our card"); 
    send_card();
    NEXT_CARD_SEND_TIME = now + NEIGHBOR_CARD_SEND_INTERVAL + 
      rand() % NEIGHBOR_CARD_SEND_INTERVAL_DEV;
  }
}

/**
@brief Отправляет запрос соседям
@detail Функция делает это не чаще чем раз за NEIGHBOR_CARD_REQ_INTERVAL
 с девиацией NEIGHBOR_CARD_REQ_INTERVAL_DEV
*/
static void send_cmd_req(){
  uint32_t now = TIC_GetUptime();
  if (now < NEXT_CARD_REQ_TIME)
    return;
  
  NEXT_CARD_REQ_TIME = now + NEIGHBOR_CARD_REQ_INTERVAL +
      rand()%NEIGHBOR_CARD_REQ_INTERVAL_DEV;  
  
  nb_frame_req_s cmd_req;
  cmd_req.cmd_req = REQ_CMD;
  
  frame_s *fr = frame_create();
  frame_addHeader(fr, &cmd_req, NB_FRAME_REQ_SIZE);
  fr->meta.PID = PID_NP;
  fr->meta.NDST = 0xffff;
  fr->meta.NSRC = CONFIG.node_adr; 
  fr->meta.TS = 0;
  fr->meta.CH = CONFIG.sys_channel;
  fr->meta.TX_METHOD = BROADCAST;
  LOG_ON("CMD request cards send")
  RP_Send(fr); 
}

/**
@brief Проверка на наличие записей в таблице
*/
static bool is_nb_table_free(void){
  for (uint8_t i = 0; i < NB_TABLE_ITEMS; i++)
    if (NB_TABLE[i].record_active)
      return false;
  return true;
}

/**
@brief Запрос соседей об картачках
@detail Запрос формируется по времени или при отсутсвии записей
*/
static void card_send_req_task(void){
  uint32_t now = TIC_GetUptime();
  // Если долго не принимали карточки от соседей 
  if (now > LAST_TIME_CARD_RECIEVED + NEIGHBOR_THR_RECV_TIME){
    send_cmd_req();
  }
  
  // Если в таблице записей нет, то посылаем запрос
  if (is_nb_table_free()){
    send_cmd_req();
  }
}

/**
@brief Выбирает соседний узел для связи
*/
static void communication_node_chose(void){  
  uint8_t best_index; 
  int best_weight = -1;
  bool found = false;
  
  // Ищем самую лучшую запись
  for (uint8_t i = 0; i < NB_TABLE_ITEMS; i++)
    if (NB_TABLE[i].record_active){
      if (NB_TABLE[i].weight > best_weight){
        best_weight = NB_TABLE[i].weight ;
        best_index = i;
        found = true;
       }
    }
  
  // Не нашли записей
  if (!found){
    COMM_NODE = NULL;
    return;
  }
  
  // Указываем лучший узел для связи с шлюзом
  COMM_NODE = &NB_TABLE[best_index];
}

/**
@brief Рабочее время модуля
*/
static void NP_TimeAlloc_HNDL(void){
  static bool need_reset = true;
  // Если сеть неактивна
  if (!SY_is_synced()){
    need_reset = true;
    return;
  }
  
  if (need_reset){
    NP_Reset();
    need_reset = false;
  }
  
  deactivate_unused_records();
  card_send_task();
  card_send_req_task();
  communication_node_chose();
  // TODO НУЖНО КОНТРЛИРОВАТЬ ИЗМЕНЕНИЯ НАШИХ СЛОТОВ и ОПОВЕЩАТЬ СОСЕДЕЙ 
}
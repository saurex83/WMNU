#include "nwdebuger.h"
#include "frame.h"
#include "LLC.h"
#include "config.h"
#include "TIC.h"
#include "stdlib.h"
#include "sync.h"
#include "MAC.h"
#include "neighnbor.h"

/**
@file
@detail Модуль контролирует содержимое CONFIG.ts_slots, CONFIG.ch_slots.
*/

// Определения

// Локальные функции
static void TB_TimeAlloc_HNDL(void);
static void open_slots(uint8_t index, uint8_t ts, uint8_t ch);
static void close_slots(uint8_t index);
static void random_pair(uint8_t *ts, uint8_t *ch);

// Глобальные функции
void TB_Init(void);
void TB_Reset(void);
void TB_Receive_HNDL(meta_s *meta);

// Локальные переменные
static uint32_t FORBIDDEN_TIME; //!< Время после которого модуль активируется
static uint8_t OPENED_SLOTS; //! Количество открытых слотов 

void TB_Init(void){
  LLC_TimeAlloc(TB_TimeAlloc_HNDL);
  TB_Reset();
}

static void close_slots(uint8_t index){
    CONFIG.ts_slots[index] = 0;
    CONFIG.ch_slots[index] = 0;  
    MAC_CloseRXSlot(CONFIG.ts_slots[index]);
}

static void open_slots(uint8_t index, uint8_t ts, uint8_t ch){
    CONFIG.ts_slots[index] = ts;
    CONFIG.ch_slots[index] = ch;  
    MAC_OpenRXSlot(ts, ch);
    LOG_ON("Open TS %d, CH %d, index %d",ts, ch, index);
}

static void random_pair(uint8_t *ts, uint8_t *ch){
  *ts = 2 + rand()%(50-2); // Временые слоты от 2 до 49
  *ch = 11 + rand()%(16-1); // Частотные каналы от 11 до 27 (16 штук) 
}

void TB_Reset(void){
  // Очистим всю информацию
  for (uint8_t i = 0; i < MAX_OPEN_SLOTS; i++)
    close_slots(i);
  FORBIDDEN_TIME = 0;
  OPENED_SLOTS = 0;
}

/**
@brief ищет и открывает новый слот на основе таблици соседей
*/
static void find_and_open_slot(void){  
  uint8_t ch, ts;
  // Проверим сколько их открыто
  if (OPENED_SLOTS >= MAX_OPEN_SLOTS)
    return;
  
  while(true){ // Генерируем пару
    random_pair(&ts, &ch);
    if (!NP_is_pair_exist(ts, ch))
      break;
  }
  OPENED_SLOTS++;
  open_slots(OPENED_SLOTS - 1, ts, ch);
}

/**
@brief Рабочее время модуля
*/
static void TB_TimeAlloc_HNDL(void){     
  static bool need_reset = true;
  // Если сеть неактивна
  if (!SY_is_synced()){
    MAC_CloseRXSlot(0);
    need_reset = true;
    return;
  }    
  
  // После входа в синхронизацию очистим настройки
  // и определим задержку после которой можно настроаивать слоты.
  // Задержка нужно что бы протокол соседей успел кого нибудь найти
  uint32_t now = TIC_GetUptime();
  if (need_reset){
    TB_Reset();
    FORBIDDEN_TIME = now + TB_START_DELAY + rand()%TB_START_DEV;
    // Так как мы синхронизировались, нужно открыть 0 слот на системной частоте
    MAC_OpenRXSlot(0, CONFIG.sys_channel);
    need_reset = false;
  }

  // Ожидаем окончания запрещенного интервала
  if (now < FORBIDDEN_TIME)
    return;
  
  // С этого места балансер выполняет свою работу.
  
  // Первым делом мы должны открыть хоть один слот
  if (OPENED_SLOTS == 0)
    find_and_open_slot();
  
  // TODO динамическое управление балансировкой
}


/**
@brief Обработка пакета 
*/
void TB_Receive_HNDL(meta_s *meta){
  
  // Meta удалять не нада, она на стеке живет
}
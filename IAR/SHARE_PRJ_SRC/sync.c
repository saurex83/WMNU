#include "stdlib.h"
#include "basic.h"
#include "frame.h"
#include "string.h"
#include "nwdebuger.h"
#include "coder.h"
#include "TIC.h"
#include "RADIO.h"
#include "NTMR.h"
#include "config.h"
#include "ioCC2530.h" // ОТЛАДДКА

void SY_Init(void);
void SY_Reset(void);
void SY_setIV(void* ptr_IV);
void SY_setKEY(void* ptr_KEY);
bool SY_SYNC_NETWORK(uint16_t *panid,uint16_t timeout);
void SY_Enable(bool en);
uint32_t SY_sync_sended(void);

static void SY_TS1_HNDL_MASTER(void);
static void SY_TS1_HNDL_SLAVE(void);
static void SY_TIME_ALLOC_SLAVE(void);
static void SY_TIME_ALLOC_MASTER(void);
static void BitRawDecrypt(uint8_t *src, uint8_t size);
static void BitRawCrypt(uint8_t *src, uint8_t size);
static frame_s* get_sync(uint16_t timeout);
static bool send_sync(void);

static uint32_t LAST_SYNC_TIME = 0; //!< Время последней синхр.
static uint32_t NEXT_SYNC_TIME = 0; //!< Время следующей синхр.
static uint32_t SYNC_SENDED = 0; //!< Количество успешно ретранслированых пакетов
static bool NEED_SEND_SYNC = false; //!< Нужно отослать синхропакет
static bool SY_ENABLE_MODULE = false;

// Ключ потокового шифрования и вектор иницилизации
static uint8_t KEY[16] = DEFAULT_KEY;
static uint8_t IV[16] = DEFAULT_IV;
//@brief Точное время приема/передачи пакета синхронизации времени
// Расчитывается при иницилизации
static uint16_t SYNC_ACCURATE_NETWORK_TIME; 

#define SYNC_TS 1 //!< Номер временного слота синхросигнала
#define SYNC_RECV_TIMEOUT 2 // Время ожидания приема пакета в мс
#define SYNC_TIMEOUT 110//!< Время в сек после которого сеть не синхронна

typedef struct // Формат структуры пакета синхронизации
{
    uint16_t panid;
    uint32_t rtc;
    uint32_t magic;
} __attribute__((packed)) SYNC_s;

#define MAGIC 0x19833891 // Проверка что пакет действительно sync
#define RAND_SYNC_RX_DELAY  9 // Фиксированое время приема rand()%10 + 5
#define RAND_SYNC_TX_DELAY  10 // Фиксированное время передачи сигнала
// Максимальное отклонение при приеме sync в тактах сети
#define SYNC_TIME_DRIFT 35  

#ifdef GATEWAY
  #define SYNC_MASTER     // Если определено, то узел является шлюзом
#endif

void SY_Init(void)
{
  SYNC_SENDED = 0;
  LAST_SYNC_TIME = 0;
  NEXT_SYNC_TIME = 0;
  SY_ENABLE_MODULE = false;
  SYNC_ACCURATE_NETWORK_TIME = TIC_SlotTime(SYNC_TS) + TIC_SlotActivityTime()/2;
#ifdef SYNC_MASTER
  TIC_SetTS1Callback(SY_TS1_HNDL_MASTER);
  TIC_SetSyncTimeAllocCallback(SY_TIME_ALLOC_MASTER);
#else
  TIC_SetTS1Callback(SY_TS1_HNDL_SLAVE);
  TIC_SetSyncTimeAllocCallback(SY_TIME_ALLOC_SLAVE);
#endif
}

void SY_Reset(void)
{
  SYNC_SENDED = 0;
  LAST_SYNC_TIME = 0;
  NEXT_SYNC_TIME = 0;
  SY_ENABLE_MODULE = false;
}

void SY_Enable(bool en)
{
  SY_ENABLE_MODULE = en;
}

/**
@brief Количество успешно переданных пакетов синхронизации
@return Количечтво пакетов
*/
uint32_t SY_sync_sended(void)
{
  return SYNC_SENDED;
}

/**
@brief Установить вектор иницилизации для шифрования
@param[in] ptr_IV указатель на 16 байтный вектор иницилизации
*/
void SY_setIV(void* ptr_IV)
{
  memcpy(IV, ptr_IV, 16);
}

/**
@brief Установить ключ шифрования
@param[in] ptr_KEY указатель на 16 байтный ключ
*/
void SY_setKEY(void* ptr_KEY)
{
  memcpy(KEY, ptr_KEY, 16);
}

/**
@brief Функция вызывается после каждого завершенного слота времени
@detail Детектирует потерю синхронизации, активирует процесс синхронизации
 активирует слот для ретрансляцию сигнала
*/
static void SY_TIME_ALLOC_SLAVE(void)
{ 
  if (!SY_ENABLE_MODULE) // Модуль отключен
    return;  
  
  // Потеря синхронизации
  if ( (TIC_GetUptime() - LAST_SYNC_TIME) > SYNC_TIMEOUT)
  {
      LOG_ON("Network out of sync.");
      return;
  }
  
  if (TIC_GetRXState(SYNC_TS)) // Если прием уже активен
    return;
  
  // Ретрансляция синхропакета
  if (NEED_SEND_SYNC)
  {
    LOG_OFF("Need to send resync");
    TIC_SetRXState(SYNC_TS, true);
    return;
  }
  
  // Необходимо начать процесс синхронизации
  if (TIC_GetUptime() > NEXT_SYNC_TIME)
  {
    LOG_ON("Begin resync");
    P1_0 = !true; 
    TIC_SetRXState(SYNC_TS, true);
  }
}

/**
@brief Функция вызывается после каждого завершенного слота времени
@detail Запускает процесс передачи синхросигнала в равные промежутки времени
*/
static void SY_TIME_ALLOC_MASTER(void)
{ 
  static uint32_t sync_send_time = 0;
 
  if (!SY_ENABLE_MODULE) // Модуль отключен
  {
    sync_send_time = 0;
    return;  
  }
  
  if (TIC_GetTXState(SYNC_TS)) // Если передача уже активна.
    return;
  
  uint32_t now = TIC_GetUptime();
  if ( now < sync_send_time) // Если время новой передачи не наступило
    return;
  
  TIC_SetTXState(SYNC_TS,true); // Разрешаем передачу
   
   // Определяем следующее время передачи
   sync_send_time = now + RAND_SYNC_TX_DELAY;
}

/**
@brief Вызывает при активности TS1 RX или TX
@detail Выполняет две функции: синхронизацию с сетью или передачу синхр.
*/
static void SY_TS1_HNDL_SLAVE(void)
{
  if (!SY_ENABLE_MODULE) // Модуль отключен
    return;
    
  // Нужно отослать синхропакет
  if (NEED_SEND_SYNC)
  {
    if (send_sync())
      SYNC_SENDED++;
    LOG_ON("Resync TX. CNT=%d",(uint16_t)SYNC_SENDED);
    NEED_SEND_SYNC = false;
    TIC_SetRXState(SYNC_TS, false);
  }
  
  // Ждем начала точного времени начала передачи сигнала заранее
  // Время в тактах сети
  NT_WaitTime(SYNC_ACCURATE_NETWORK_TIME - SYNC_TIME_DRIFT);
 
  // Время в мс
  frame_s *fr_SYNC = get_sync(SYNC_RECV_TIMEOUT); 
  
  if (!fr_SYNC)
    return;
  
  SYNC_s *sync;
  sync = (SYNC_s*)fr_SYNC->payload;
  
  // Проверяем принадлежность пакета
  if (( sync->panid != CONFIG.panid) && (sync->magic != MAGIC))
  {
    frame_delete(fr_SYNC);
    return;
  }
  
  // Синхронизируемся
  // Время прошедшее с момента приема пакета в тактах сети
  uint16_t delta = TIC_GetTimer() - fr_SYNC->meta.TIMESTAMP;
  TIC_SetTimer(SYNC_ACCURATE_NETWORK_TIME + delta);
  TIC_SetRTC(sync->rtc);
  
  LAST_SYNC_TIME = TIC_GetUptime();
  NEXT_SYNC_TIME = LAST_SYNC_TIME + RAND_SYNC_RX_DELAY; 
  TIC_SetRXState(SYNC_TS, false);
  LOG_ON("Node Synced. TS=%d, AT=%d, DEL=%d, AD=%d, RTC=%d, NRTC=%d, SRTC=%d",
         fr_SYNC->meta.TIMESTAMP,SYNC_ACCURATE_NETWORK_TIME, delta,
         SYNC_ACCURATE_NETWORK_TIME - fr_SYNC->meta.TIMESTAMP, 
         (uint16_t)LAST_SYNC_TIME,
         (uint16_t)NEXT_SYNC_TIME,
         (uint16_t)TIC_GetRTC());
  frame_delete(fr_SYNC);
  // После приема нужно ретранслировать синхропакет
  NEED_SEND_SYNC = true; 
P1_0 = !false; // ОТЛАДКА
}

/**
@brief Вызывает при активносм TS1 RX или TX
@detail Формирует кадр синхронизации и передает его в строго определенное время
*/
static void SY_TS1_HNDL_MASTER(void)
{
  if (!SY_ENABLE_MODULE) // Модуль отключен
    return;  
   
P1_0 = !true; //ОТЛАДКА  
  if (send_sync())
    SYNC_SENDED++;
  
  LOG_ON("Sync send."); 
P1_0 = !false; //ОТЛАДКА
  
  TIC_SetTXState(SYNC_TS, false);
}

/**
@brief Создать и отправить синхропакет
*/
static bool send_sync(void)
{
  SYNC_s sync;
  sync.panid = CONFIG.panid;
  sync.rtc = TIC_GetRTC();
  sync.magic = MAGIC;
  
  frame_s *fr_SYNC = frame_create();
  frame_addHeader(fr_SYNC, &sync, sizeof(SYNC_s));
  fr_SYNC->meta.SEND_TIME = SYNC_ACCURATE_NETWORK_TIME; // время в тактах сети. ((1/32768)*164)*1000
    
#ifdef RARIO_STREAM_ENCRYPT
  BitRawCrypt(fr_SYNC->payload, fr_SYNC->len);
#endif
  
  RI_SetChannel(CONFIG.sync_channel);
  bool res = RI_Send(fr_SYNC);
  
  frame_delete(fr_SYNC);
  return res;
}

/**
@brief Прием пакет синхронизации
@param[in] Время ожидания в мс
@return Указатель на пакет или NULL
*/
static frame_s* get_sync(uint16_t timeout)
{
  RI_SetChannel(CONFIG.sync_channel);
  frame_s *fr_SYNC = RI_Receive(timeout);
  
  // Если пакета нет, выходим из обработчика
  if (fr_SYNC == NULL)
    return NULL;
  
  // Проверим размер пакета
  if (fr_SYNC->len != sizeof(SYNC_s))
  {
    frame_delete(fr_SYNC);
    return NULL;
  }
  
  #ifdef RARIO_STREAM_ENCRYPT
  BitRawDecrypt(fr_SYNC->payload, fr_SYNC->len);
  #endif
  return fr_SYNC;
}

/**
@brief Процедура синхронизации с сетью
@detail Процедура изет сеть в течении времени timeout и синхронизируется с ней
@param[in] timeout время в милисекундах
@param[out] panid указатель на идентификатор найденой сети
@return true если сеть найдета
*/
bool SY_SYNC_NETWORK(uint16_t *panid,uint16_t timeout)
{
  // TODO Нужно обнулить настройки всех модулей
  // Или об этом будет заботиться верхний уровень?
  TIC_CloseAllState();
  
  RI_SetChannel(CONFIG.sync_channel);
  
  TimeStamp_s begin, end;
  TIM_TimeStamp(&begin);
  TIM_TimeStamp(&end);
  
  bool net_found = false;
  uint32_t passed = 0;
  frame_s *fr_SYNC;
  SYNC_s *sync;
  uint32_t timeout_us = timeout * 1000;
  
  while (passed < timeout_us)
  {
    fr_SYNC = get_sync(timeout);
    if (fr_SYNC == NULL)
    {
      passed = TIM_passedTime(&begin, &end);
      continue;
    }
    
    // Приняли какойто пакет, декодируем.
    sync = (SYNC_s*)fr_SYNC->payload;
    
    if (sync->magic != MAGIC)
    { // Проверим что пакет именно тот а не эфирный мусор
      frame_delete(fr_SYNC);
      passed = TIM_passedTime(&begin, &end);
      continue;
    }   
    
    // Возвращаем результат
    *panid = sync->panid;
    CONFIG.panid = sync->panid;
    net_found = true;
    
    // Синхронизируемся с сетью
    // Время прошедшее с момента приема пакета в тактах сети
    uint16_t delta = TIC_GetTimer() - fr_SYNC->meta.TIMESTAMP;
    TIC_SetTimer(SYNC_ACCURATE_NETWORK_TIME + delta);
    TIC_SetRTC(sync->rtc);
    LAST_SYNC_TIME = TIC_GetUptime();
    NEXT_SYNC_TIME = LAST_SYNC_TIME + RAND_SYNC_RX_DELAY; 
    frame_delete(fr_SYNC);
    break;
  } 
  
  return net_found;
}

/*!
\brief Расшифровка область памяти
\param[in,out] *src Указатель на начало области дешифрования
\param[in] size Размер расшифруемых данных
*/
static void BitRawDecrypt(uint8_t *src, uint8_t size)
{
  AES_StreamCoder(false, src, src, KEY, IV, size);
}

/*!
\brief Шифрует область памяти
\param[in,out] *src Указатель на начало области шифрования
\param[in] size Размер шифруемых данных
*/
static void BitRawCrypt(uint8_t *src, uint8_t size)
{
  AES_StreamCoder(true, src, src, KEY, IV, size);
}


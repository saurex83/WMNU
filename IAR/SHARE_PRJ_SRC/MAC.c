#include "MAC.h"
#include "frame.h"
#include "string.h"
#include "nwdebuger.h"
#include "coder.h"
#include "TIC.h"
#include "stdlib.h"
#include "basic.h"
#include "NTMR.h"  // ОТЛАДКА
// Обработчики прерываний
static void MAC_RX_HNDL(uint8_t TS);
static void MAC_TX_HNDL(uint8_t TS);
static void (*RXCallback)(frame_s *fr);
static void BitRawDecrypt(uint8_t *src, uint8_t size);
static void BitRawCrypt(uint8_t *src, uint8_t size);
static uint8_t xor_calc(frame_s *fr);

// Публичные методы
void MAC_Init(void);
void MAC_Reset(void);
void MAC_Enable(bool en);
void MAC_OpenRXSlot(uint8_t TS, uint8_t CH);
void MAC_CloseRXSlot(uint8_t TS);
void MAC_Send(frame_s *fr, uint8_t attempts);
bool MAC_ACK_Send(frame_s *fr);
void MAC_SetRXCallback(void (*fn)(frame_s *fr));
bool MAC_GetTXState(uint8_t TS);
bool MAC_GetRXState(uint8_t TS);

// Ключ потокового шифрования и вектор иницилизации
static uint8_t KEY[16] = DEFAULT_KEY;
static uint8_t IV[16] = DEFAULT_IV;
  
#define RECV_TIMEOUT 2500 // Время ожидания приема пакета в мкс с начала слота
#define ACK_RECV_TIMEOUT 1000 // Время ожидания приема подтверждения в мкс
#define TX_DELAY 1*Tmsec // Смещение при передаче пакета. Защита от девиации времени

typedef struct // Формат структуры пакета ACK
{
  uint16_t CRC8;
} __attribute__((packed)) ACK_s;

typedef struct
{
 struct 
 {
   bool enable;
   uint8_t attempts;
   uint8_t CH;
   frame_s *fr;
 } __attribute__((packed)) TX;
 
 struct
 {
  bool enable; 
  uint8_t CH;
 } __attribute__((packed)) RX;
} __attribute__((packed)) MACSState_s; 

static bool MAC_ENABLE_MODULE = false; //!< Модуль активен


// Таблица состояний слотов приема/передачи
MACSState_s MACSlotTable[50];

/**
@brief Установить вектор иницилизации для шифрования
@param[in] ptr_IV указатель на 16 байтный вектор иницилизации
*/
void MAC_setIV(void* ptr_IV)
{
  memcpy(IV, ptr_IV, 16);
}

/**
@brief Установить ключ шифрования
@param[in] ptr_KEY указатель на 16 байтный ключ
*/
void MAC_setKEY(void* ptr_KEY)
{
  memcpy(KEY, ptr_KEY, 16);
}

/**
@brief Иницилизация MAC
@detail Инизилизирует TIM, TIC, RI, AES
*/
void MAC_Init(void)
{
  MAC_ENABLE_MODULE = false;
  TIC_SetRXCallback(MAC_RX_HNDL);
  TIC_SetTXCallback(MAC_TX_HNDL);  
  memset(MACSlotTable, 0x00, 50*sizeof(MACSState_s));
}

/**
@brief Сброс настроек модуля.
@detail Удаяляет все пакеты для передачи. Активным остается TS1 обработчик sync
*/
void MAC_Reset(void)
{
  MAC_ENABLE_MODULE = false;
  // Удаляем все пакеты на передачу
  for (uint8_t i = 0; i < 50; i++)
    if (MACSlotTable[i].TX.enable)
      frame_delete(MACSlotTable[i].TX.fr);
  
  memset(MACSlotTable, 0x00, 50*sizeof(MACSState_s));
}

/**
@brief Открыть временой слот приема данных
@param[in] TS номер слота от 0 до 49
@param[in] CH номер радиоканала
*/
void MAC_OpenRXSlot(uint8_t TS, uint8_t CH)
{
    ASSERT(TS < 50 || TS !=0 );
    MACSlotTable[TS].RX.enable = true;
    MACSlotTable[TS].RX.CH = CH;
    TIC_SetRXState(TS, true);
}

/**
@brief Закрыть временой слот приема данных
@param[in] TS номер слота от 0 до 49
*/
void MAC_CloseRXSlot(uint8_t TS)
{
  ASSERT(TS < 50 || TS !=0);
  MACSlotTable[TS].RX.enable = false;
  TIC_SetRXState(TS, false);
}

/**
@brief Поставить пакет в таблицу отправки сообщений
@param[in] fr указатель на пакет
@param[in] attempts количество попыток
*/
void MAC_Send(frame_s *fr, uint8_t attempts)
{
    ASSERT(fr != NULL);
    ASSERT(attempts != 0);
    ASSERT(fr->meta.TS != 0);
    
    uint8_t TS = fr->meta.TS; 
    MACSlotTable[TS].TX.attempts = attempts;
    MACSlotTable[TS].TX.CH = fr->meta.CH;
    MACSlotTable[TS].TX.enable = true;
    MACSlotTable[TS].TX.fr = fr;
    
    #ifdef RARIO_STREAM_ENCRYPT
    BitRawCrypt(fr->payload, fr->len);
    #endif
    
    TIC_SetTXState(TS, true);
}

void MAC_Enable(bool en)
{
  MAC_ENABLE_MODULE = en;
}

/**
@brief Расчитывает CRC8 код
@param[in] fr указатель на кадр
@return CRC8
*/
static uint8_t xor_calc(frame_s *fr)
{
  uint8_t crc = 0x34; // Начальное значение
  uint8_t *val = fr->payload;
  
  for (uint8_t i = 0; i < fr->len; i++)
    crc ^= val[i];
  return crc;
}

/**
@brief Посылает подтверждение приема пакета
@param[in] fr указатель на кадр который нужно подтвердить
@return true если передача подтверждения успешна (канал свободен)
*/
static bool MAC_ACK_Send(frame_s *fr)
{
  static ACK_s pACK;
  frame_s *fr_ACK;
  
  // Создаем подтверждение кадра
  pACK.CRC8 = xor_calc(fr);
  
  // Создаем кадр для отправки
  fr_ACK = frame_create();
  frame_addHeader(fr_ACK, &pACK, sizeof(ACK_s));
  fr_ACK->meta.SEND_TIME = 0;
  
 
  bool res = RI_Send(fr_ACK);
  frame_delete(fr_ACK);
  return res;
}

/**
@brief Ожидает прием пакета подтверждения
@param[in] fr указатель на пакет подтверждение которого ожидаем 
@return true если приняли подтверждение
*/
static bool MAC_ACK_Recv(frame_s *fr)
{
  frame_s *fr_ACK = RI_Receive(ACK_RECV_TIMEOUT);
  
  // Если пакета нет, выходим из обработчика
  if (fr_ACK == NULL)
    return false;
  
  // Проверим размер пакета
  if (fr_ACK->len != sizeof(ACK_s))
  {
    frame_delete(fr_ACK);
    return false;
  }
    
  ACK_s *ptrACK;
  ptrACK = (ACK_s*)fr_ACK->payload;
  
  uint8_t crc8 = xor_calc(fr);
  uint8_t ack_crc8 = ptrACK->CRC8;
  frame_delete(fr_ACK);
  
  if (crc8 == ack_crc8)
    return true;
  return false;
}

/**
@brief Устанавливает функцию обратного вызова при приеме пакета
@param[in] fn указатель на функцию обработчик
*/
void MAC_SetRXCallback(void (*fn)(frame_s *fr))
{
  RXCallback = fn;
}

/**
@brief Состояние слота TS передача
@return возвращает true, если слот содержит пакет для передачи
*/
bool MAC_GetTXState(uint8_t TS)
{
  ASSERT(TS < 50  || TS !=0 );
  return MACSlotTable[TS].TX.enable;
}

/**
@brief Состояние слота TS приема
@return возвращает true, если слот принимает пакет
*/
bool MAC_GetRXState(uint8_t TS)
{
  ASSERT(TS < 50  || TS !=0);
  return MACSlotTable[TS].RX.enable;
}

/**
@brief Обработчик слота приема пакета
@detail При необходимости подтвеждает пакет
@param[in] TS номер временного слота
*/
static void MAC_RX_HNDL(uint8_t TS)
{
  ASSERT(TS < 50  || TS !=0);

  if (!MAC_ENABLE_MODULE) // Модуль откючен
    return;
  
  // Задержка перед приемом. Прием начинается через 427мкс и заканчивается в
  // 3.2 мс от начала слота. Длительность 2.8мс. Длятельность больше 2мс так
  // как SDF приходит позже чем фактическая передача на 400мкс
  TIM_delay(200);
  RI_SetChannel(MACSlotTable[TS].RX.CH);
  frame_s *fr = RI_Receive(RECV_TIMEOUT);

//volatile uint16_t DBGT= NT_GetTime();  
  
 // frame_s *fr = RI_Receive(2000);
  
//volatile uint16_t DBGT2= NT_GetTime();   
  // Если пакета нет, выходим из обработчика
  if (fr == NULL)
    return;
  
  // Пакеты во временные слоты 1..49 требуют подтверждения
  // Слоты 0 и 1 для швс и синхронизации соответсвенно
  if (TS > 1)
    MAC_ACK_Send(fr);
 
  #ifdef RARIO_STREAM_ENCRYPT
  BitRawDecrypt(fr->payload, fr->len);
  #endif
  
  RXCallback(fr);   // Передаем пакет на дальнейшую обработку
  // Удаление пакета не наша забота
}

/**
@brief Обработчик слота пердачи пакета
@detail После отправки пакета требуется подтверждение приема если meta.ACK = 1.
 После отправки ждем приема ACK. Проверка подлинности ACK происходит с помощью
 обратного вызова isACK_OK(*fr, *fr_ACK). Результат true или false.
 Передача начинается через 1.5 мс.
@param[in] TS номер временного слота
*/
static void MAC_TX_HNDL(uint8_t TS)
{
  ASSERT(TS < 50  || TS !=0);
  
  if (!MAC_ENABLE_MODULE) // Модуль отключен
    return;
  
  // По ошибки вызвали. Такого быть не должно, но подстрахуемся.
  if ((!MACSlotTable[TS].TX.enable) || (MACSlotTable[TS].TX.attempts == 0)) 
  {
    TIC_SetTXState(TS, false);
    return;
  }
  
  RI_SetChannel(MACSlotTable[TS].TX.CH); // Устанавливаем канал передачи
//volatile uint16_t DBGT = 5;
//DBGT= NT_GetTime();
  // Задержка перед передачей данных необходима для учета отклонения
  // времени между узлами из-за рассинхронизации узлов.
  // Задержка составляет 1.5 мс (49 тактов сети)
  // В delay число 918, оно учитывает время включения радио, контроля CCA
  // и начал передачи будет чере 1.5мс
//volatile uint16_t DBGT= NT_GetTime();
    TIM_delay(918);
  // Пробуем передать данные
  bool tx_success = RI_Send(MACSlotTable[TS].TX.fr); 
  bool send_success = false;  
  
  LOG_OFF("RI_Send = %d, CH = %d, TS = %d\r\n",
      tx_success, MACSlotTable[TS].TX.CH, TS);
//DBGT= NT_GetTime();    
  if (tx_success)
  {
    if (TS > 1) // требуется подтверждение ACK
    {
      if (MAC_ACK_Recv(MACSlotTable[TS].TX.fr))
          send_success = true;
    }
    else // не требуеться подтверждение
      send_success = true;
  }
  

  if (send_success) // В случаи успеха удаляем данные и закрываем слоты 
  { 
    frame_delete(MACSlotTable[TS].TX.fr);
    MACSlotTable[TS].TX.enable = false;
    TIC_SetTXState(TS, false);      
  }
  else // Уменьшаем счетчик попыток передачи и удаляем пакет при достижении 0.
  {
    MACSlotTable[TS].TX.attempts --;
    if (MACSlotTable[TS].TX.attempts == 0)
      {
        frame_delete(MACSlotTable[TS].TX.fr);
        MACSlotTable[TS].TX.enable = false;
        TIC_SetTXState(TS, false);  
      }
  }
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


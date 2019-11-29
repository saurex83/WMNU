#include "MAC.h"
#include "frame.h"
#include "string.h"
#include "nwdebuger.h"
#include "coder.h"
#include "TIC.h"

// Обработчики прерываний
static void MAC_RX_HNDL(uint8_t TS);
static void MAC_TX_HNDL(uint8_t TS);
static void (*RXCallback)(frame_s *fr);
bool (*isACK_OK)(frame_s *fr, frame_s *fr_ack);

// Публичные методы
void MAC_Init(void);
void MAC_OpenRXSlot(uint8_t TS, uint8_t CH);
void MAC_CloseRXSlot(uint8_t TS);
void MAC_Send(frame_s *fr, uint8_t attempts);
bool MAC_ACK_Send(frame_s *fr);
void MAC_SetRXCallback(void (*fn)(frame_s *fr));
void MAC_Set_isACK_OK_Callback(bool(*fn)(frame_s *fr, frame_s *fr_ack));
bool MAC_GetTXState(uint8_t TS);
bool MAC_GetRXState(uint8_t TS);

// Ключ потокового шифрования и вектор иницилизации
uint8_t KEY[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
uint8_t IV[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  
#define RECV_TIMEOUT 3000UL // Время ожидания приема пакета в мкс
#define ACK_RECV_TIMEOUT 1000UL // Время ожидания приема подтверждения в мкс
// Задержка перед приемом ACK в мкс если данные шифруются
#define DELAY_BEFORE_ACK_RECV_CRYPT 2000UL 
// Задержка перед приемом ACK в мкс если данные не шифруются
#define DELAY_BEFORE_ACK_RECV_NOCRYPT 1000UL

#define RARIO_STREAM_ENCRYPT true // Шифрование данных включенно 

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


// Таблица состояний слотов приема/передачи
MACSState_s MACSlotTable[50];


/**
@brief Установка обработчика функции  isACK_OK
@param[in] fn(frame_s *fr, frame_s *fr_ack) указатель на функцию
*/
void MAC_Set_isACK_OK_Callback(bool(*fn)(frame_s *fr, frame_s *fr_ack))
{
  isACK_OK = fn;
}

/**
@brief Иницилизация MAC
@detail Инизилизирует TIM, TIC, RI, AES
*/
void MAC_Init(void)
{
  TIM_init();
  TIC_Init();
  RI_init();
  AES_init();
  
  TIC_SetRXCallback(MAC_RX_HNDL);
  TIC_SetTXCallback(MAC_TX_HNDL);
  RI_StreamCrypt(RARIO_STREAM_ENCRYPT);
  RI_setKEY(KEY);
  RI_setIV(IV);  
  memset(MACSlotTable, 0x00, 50*sizeof(MACSState_s));
}

/**
@brief Открыть временой слот приема данных
@param[in] TS номер слота от 0 до 49
@param[in] CH номер радиоканала
*/
void MAC_OpenRXSlot(uint8_t TS, uint8_t CH)
{
    ASSERT_HALT(TS < 50, "Incorrect TS");
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
  ASSERT_HALT(TS < 50, "Incorrect TS");
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
    ASSERT_HALT(fr != NULL, "*fr null");
    ASSERT_HALT(attempts != 0, "attempts = 0");
    
    uint8_t TS = fr->meta.TS; 
    MACSlotTable[TS].TX.attempts = attempts;
    MACSlotTable[TS].TX.CH = fr->meta.CH;
    MACSlotTable[TS].TX.enable = true;
    MACSlotTable[TS].TX.fr = fr;
    TIC_SetTXState(TS, true);
}

/**
@brief Посылает подтверждение приема пакета
@param[in] fr указатель на кадр подтверждения
*/
bool MAC_ACK_Send(frame_s *fr)
{
  RI_SetChannel(fr->meta.CH);
  bool res = RI_Send(fr);
  frame_delete(fr);
  return res;
}

/**          *********** TODO ************ прием из Ethernet протокола
@brief Посылает подтверждение приема пакета
@param[in] fr указатель на кадр подтверждения
*/
frame_s* MAC_ACK_Recv(void)
{
  // ACK RECV TIMEOUT
  //RI_SetChannel(fr->meta.CH);
  //RI_Send(fr);
  //frame_delete(fr);
  return NULL;
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
  ASSERT_HALT(TS < 50, "Incorrect TS");
  return MACSlotTable[TS].TX.enable;
}

/**
@brief Состояние слота TS приема
@return возвращает true, если слот принимает пакет
*/
bool MAC_GetRXState(uint8_t TS)
{
  ASSERT_HALT(TS < 50, "Incorrect TS");
  return MACSlotTable[TS].RX.enable;
}

/**
@brief Обработчик слота приема пакета
@detail Отправкой подтвеждения приема пакета занимается ethernet протокол
@param[in] TS номер временного слота
*/
static void MAC_RX_HNDL(uint8_t TS)
{
  ASSERT_HALT(TS < 50, "Incorrect TS");
  
  RI_SetChannel(MACSlotTable[TS].RX.CH);
  frame_s *fr = RI_Receive(RECV_TIMEOUT);
  
  // Если пакета нет, выходим из обработчика
  if (fr == NULL)
    return;
  
  RXCallback(fr);   // Передаем пакет на дальнейшую обработку
  frame_delete(fr); // Удаляем пакет
}

/**
@brief Обработчик слота пердачи пакета
@detail После отправки пакета требуется подтверждение приема если meta.ACK = 1.
 После отправки ждем приема ACK. Проверка подлинности ACK происходит с помощью
 обратного вызова isACK_OK(*fr, *fr_ACK). Результат true или false.
@param[in] TS номер временного слота
*/
static void MAC_TX_HNDL(uint8_t TS)
{
  ASSERT_HALT(TS < 50, "Incorrect TS");
  
  // По ошибки вызвали. Такого быть не должно, но подстрахуемся.
  if ((!MACSlotTable[TS].TX.enable) || (MACSlotTable[TS].TX.attempts == 0)) 
    TIC_SetTXState(TS, false);
   
  RI_SetChannel(MACSlotTable[TS].TX.CH); // Устанавливаем канал передачи

  // Пробуем передать данные
  bool tx_success = RI_Send(MACSlotTable[TS].TX.fr); 
  bool send_success = false;  
  
  LOG(MSG_OFF | MSG_INFO | MSG_TRACE, "RI_Send = %d, CH = %d, TS = %d\r\n",
      tx_success, MACSlotTable[TS].TX.CH, TS);
    
  // Если отправка была успешна и требуется прием подтверждения ACK
  if (tx_success && MACSlotTable[TS].TX.fr->meta.ACK)
  {
    // TODO ждем пакета ACK
    // Задержка отправки подтверждения пока что не известна.
    // Если есть шифрование это на 1 мс дольше чем без него
    // Можно подумать над тем , что бы пакет ACK был не ETH формата.
    // К примеру ACK = LEN, FRAME_LEN, FCS1, FCS2 и он существовал
    // на уровне MAC и не использовал ETH. Отправка ACK без шифрования.
    // Это увеличит время работы узла. Или вместо FRAME_LEN отправлять
    // FCS1, FCS2 отправленного пакета что бы его подтвердить или использовать
    // свой алгоритм расчета CRC
    
    // Если включено шифрование, то можно выключать радиоприемник для экономии
    // Шифрование данных занимает некоторое время (минимум 1 мс)
    if (RARIO_STREAM_ENCRYPT)
      TIM_delay(DELAY_BEFORE_ACK_RECV_CRYPT);
    else
      TIM_delay(DELAY_BEFORE_ACK_RECV_NOCRYPT);
    
    frame_s *fr_ACK = RI_Receive(ACK_RECV_TIMEOUT);
  
    if (fr_ACK == NULL) // Не приняли ACK
      goto LABEL_MAC_TX_HNDL_END;
    
    else // Приняли ACK
    {
      ASSERT_HALT(isACK_OK !=NULL, "isACK_OK func NULL");
     
      // Проверим является ли принятый пакет ACK подтверждением 
      // переданного пакета 
      bool isACK = isACK_OK(MACSlotTable[TS].TX.fr, fr_ACK); 
     
      if (isACK) // Пакет подтвержден
        send_success = true;
      
      frame_delete(fr_ACK); // Удаляем принятый пакет ACK
      goto LABEL_MAC_TX_HNDL_END;
    }
  }
 
  // Если отправка была успешна и НЕ требуется подтверждение ACK
  if (tx_success && !MACSlotTable[TS].TX.fr->meta.ACK) 
    send_success = true;
  
  
LABEL_MAC_TX_HNDL_END:  
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

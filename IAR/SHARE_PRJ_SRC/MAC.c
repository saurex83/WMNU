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

// Публичные методы
void MAC_Init(void);
void MAC_OpenRXSlot(uint8_t TS, uint8_t CH);
void MAC_CloseRXSlot(uint8_t TS);
void MAC_Send(frame_s *fr, uint8_t attempts);
void MAC_ACK_Send(frame_s *fr);
void MAC_SetRXCallback(void (*fn)(frame_s *fr));
bool MAC_GetTXState(uint8_t TS);
bool MAC_GetRXState(uint8_t TS);

// Ключ потокового шифрования и вектор иницилизации
uint8_t KEY[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
uint8_t IV[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  
#define RECV_TIMEOUT 3000 // Время ожидания приема пакета в мкс

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


void MAC_Init(void)
{
  TIM_init();
  TIC_Init();
  RI_init();
  AES_init();
  
  TIC_SetRXCallback(MAC_RX_HNDL);
  TIC_SetTXCallback(MAC_TX_HNDL);
  RI_StreamCrypt(true);
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
void MAC_ACK_Send(frame_s *fr)
{
  RI_SetChannel(fr->meta.CH);
  RI_Send(fr);
  frame_delete(fr);
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
  
  if (tx_success) // В случаи успеха удаляем данные и закрываем слоты 
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
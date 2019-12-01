#include "frame.h"
#include "Net_frames.h"
#include "stdint.h"
#include "nwdebuger.h"
#include "LLC.h"
#include "mem.h"

void ETH_Send(frame_s *fr);

static void ETH_RX_HNDL(frame_s *fr);
static void (*RXCallback)(frame_s *fr); //frame_s RAW_LAY, wthout ETH_LAY
static bool validate(ETH_LAY *eth);
static ETH_LAY* extract_header(frame_s *fr);
static void send_ack(ETH_LAY *eth);
static frame_s* strip_header(frame_s *fr);

/**
@brief Иницилизация модуля
*/
void ETH_Init(void)
{
  RXCallback = NULL; 
  LLC_SetRXCallback(ETH_RX_HNDL);
}

/**
@brief Установка обработчика приема пакета ETH
*/
void ETH_Set_RXCallback(void (*fn)(frame_s *fr))
{
  ASSERT_HALT(fn != NULL, "NULL pointer not allow");
  RXCallback = fn;
}

/**
@brief Обработка принятого пакет сети.
@detail После валидации пакета, уничтожается заголовок ETH и пакет передается
 на дальнейшую обработку вверх по стеку с помощью обратного вызова RXCallback
*/
static void ETH_RX_HNDL(frame_s *fr)
{
  ASSERT_HALT(fr != NULL, "fr is NULL");
  
  bool valid;
  frame_s *striped_frame;
  
  ETH_LAY* eth_h = extract_header(fr);
  
  // Извлечь заголовок не удалось
  if (eth_h == NULL)
    goto ERR_FRAME;
  
  valid = validate(eth_h);
 
  // Заголовок не верный
  if (!valid)
    goto ERR_FRAME;
 
  // Требуется отправить подтверждение приема пакета
  if (eth_h->ETH_T.bits.ACK)
    send_ack(eth_h);
  
  // Создаем новый пакет без заголовка и удаляем исходный
  striped_frame = strip_header(fr);
  frame_delete(fr);
  ASSERT_HALT(striped_frame != NULL, "Striped_frame is NULL");
  
  // Передаем обработчику выше по стеку
  ASSERT_HALT(RXCallback != NULL, "RXCallback is NULL");
  RXCallback(striped_frame);
  
  re_free(eth_h);
  return;
  
ERR_FRAME:
  frame_delete(fr);
  re_free(eth_h);
  return;
}

/**
@brief Создает новый пакет без заголовка ETH_H
@return указатель на новый frame_s*. NULL - не удалось создать пакет
*/
static frame_s* strip_header(frame_s *fr)
{
}

/**
@brief Подготавливает и отпправляет подтверждение приема пакета
*/
static void send_ack(ETH_LAY *eth)
{
 
}

static bool validate(ETH_LAY *eth)
{
  return true;
}

static ETH_LAY* extract_header(frame_s *fr)
{
  ETH_LAY* eth_h = (ETH_LAY*)re_malloc(ETH_LAY_SIZE);
  ASSERT_HALT(eth_h != NULL, "No memory");
  
  uint8_t len = frame_len(fr);
  ASSERT_HALT(len >= ETH_LAY_SIZE, "Incorrect eth size");
  
  fbuf_s *fb = frame_get_fbuf_head(fr);
  re_memcpy(eth_h, fb->payload, ETH_LAY_SIZE);
  return eth_h;
}

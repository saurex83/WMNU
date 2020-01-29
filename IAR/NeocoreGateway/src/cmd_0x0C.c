#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "MAC.h"
#include "nwdebuger.h"
#include "rx_buff.h"

static void upload_frame(frame_s *fr);
static void mem_move(uint8_t *dst, uint8_t *src, uint8_t len);

#define ARGS_SIZE sizeof(cmd_args_s)
typedef struct //!< Аргументы команды
{
  uint16_t crc16;
} cmd_args_s;


static uint8_t upload[sizeof(meta_s) + 1 + 127];

/**
@brief Отправить на PC принятый пакет из буфера RX. Сеть вкл.
*/
bool cmd_0x0C(uint8_t *cmd, uint8_t size)
{
  if (size != ARGS_SIZE){ // Размер аргументов не верен
    cmd_answer_err(ATYPE_CMD_ERR, CMD_LEN);
    return false;
  }
  
  if (!get_network_seed_status()){ // Сеть должна работать
    cmd_answer_err(ATYPE_CMD_ERR, CMD_NOSEEDING);
    return false;
  }
  
  frame_s *rx_frame = RXB_get_frame();
  
  if (rx_frame == NULL){ // В буфере ничего нет
    cmd_answer_err(ATYPE_CMD_ERR, CMD_RX_EMPTY);
    return false;
  }
  
  upload_frame(rx_frame);
 // RXB_del_frame(rx_frame);
  LOG_ON("CMD 0x0C. Upload RX frame");
  return true;
}

/**
@brief Выгрузить фрейм в uart
@detail Порядок выгрузки meta, byte len, payload[len]
*/
static void upload_frame(frame_s *fr){
/*  Заполнение массива upload
          -----------------------
          |META|LEN|   PAYLOAD  |
          -----------------------
               ^   ^<- fr.len ->^
sizeof(meta_s)-|   |
sizeof(meta_s)+1  -| 
*/
  // Пакет буду собирать в массиве upload иначе crc считать неудобно
   // Размер мета+1 байт длинны+пакет
  uint8_t answ_len = sizeof(meta_s) + 1 + fr->len; // Суммарный размер пакета
  mem_move(upload, (uint8_t*)&fr->meta, sizeof(meta_s));
  upload[sizeof(meta_s)] = fr->len; // byte len
  mem_move(&upload[sizeof(meta_s) + 1], (uint8_t*)fr->payload, fr->len);
  
  cmd_answer(ATYPE_CMD_OK, upload, answ_len);
  //cmd_answer(ATYPE_CMD_OK, upload, 10);
}

static void mem_move(uint8_t *dst, uint8_t *src, uint8_t len){
  for (uint8_t i = 0; i < len; i++)
    dst[i] = src[i];
}
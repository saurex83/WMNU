#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "MAC.h"
#include "nwdebuger.h"
#include "rx_buff.h"

#define ARGS_SIZE sizeof(cmd_args_s)
typedef struct //!< Аргументы команды
{
  uint16_t crc16;
} cmd_args_s;

/**
@brief Количество принятых пакетов в буфере RX. Сеть вкл.
*/
bool cmd_0x09(uint8_t *cmd, uint8_t size)
{
  if (size != ARGS_SIZE){ // Размер аргументов не верен
    cmd_answer_err(ATYPE_CMD_ERR, CMD_LEN);
    return false;
  }
  
  uint8_t nbItems = RXB_frame_count();
  cmd_answer(ATYPE_CMD_OK, &nbItems, sizeof(nbItems));
  
  LOG_ON("CMD 0x09. RX frame count");
  return true;
}
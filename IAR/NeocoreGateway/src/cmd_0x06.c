#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "TIC.h"
#include "nwdebuger.h"

#define ARGS_SIZE sizeof(cmd_args_s)
typedef struct //!< Аргументы команды
{
  uint8_t dont_use;
  uint16_t crc16;
} cmd_args_s;

/**
@brief перезарузка
*/
bool cmd_0x06(uint8_t *cmd, uint8_t size)
{
  
  if (size != ARGS_SIZE){ // Размер аргументов не верен
    cmd_answer_err(ATYPE_CMD_ERR, CMD_LEN);
    return false;
  }
   
  cmd_answer(ATYPE_CMD_OK, NULL, 0);
  LOG_ON("CMD 0x06. HW reload");
  //TODO АППАПРАТНАЯ перезагрузка.
  
  return true;
}
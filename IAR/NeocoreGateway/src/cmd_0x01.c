#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "nwdebuger.h"

#define ARGS_SIZE sizeof(cmd_args_s)
typedef struct //!< Аргументы команды
{
  uint16_t panid;
  uint16_t crc16;
} cmd_args_s;

/**
@brief Установить panid
*/
bool cmd_0x01(uint8_t *cmd, uint8_t size)
{
  if (size != ARGS_SIZE){ // Размер аргументов не верен
    cmd_answer_err(ATYPE_CMD_ERR, CMD_LEN);
    return false;
  }
    
  if (get_network_seed_status()){ // Сеть активна и менять ничего нельзя
    cmd_answer_err(ATYPE_CMD_ERR, CMD_SEEDING);
    return false;
  }
  
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  
  CONFIG.panid =args->panid;
  LOG_ON("CMD 0x01. Set panid");
  
  cmd_answer(ATYPE_CMD_OK, NULL, 0);
  return true;
}
#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "string.h"
#include "nwdebuger.h"

#define ARGS_SIZE sizeof(cmd_args_s)
typedef struct //!< Аргументы команды
{
  uint8_t IV[16];
  uint16_t crc16;
} cmd_args_s;

/**
@brief Загрузка IV
*/
bool cmd_0x04(uint8_t *cmd, uint8_t size)
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
  memcpy(CONFIG.stream_iv,  args->IV, 16);   
 
  LOG_ON("CMD 0x04. IV loaded");
  
  cmd_answer(ATYPE_CMD_OK, NULL, 0);
  return true;
}
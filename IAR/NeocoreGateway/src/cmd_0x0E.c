#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "MAC.h"
#include "nwdebuger.h"

#define ARGS_SIZE sizeof(cmd_args_s)
typedef struct //!< Аргументы команды
{
  uint8_t CH;
  uint16_t crc16;
} cmd_args_s;

/**
@brief Установить канал для синхросигнала. Сеть выкл.
*/
bool cmd_0x0E(uint8_t *cmd, uint8_t size)
{
  if (size != ARGS_SIZE){ // Размер аргументов не верен
    cmd_answer_err(ATYPE_CMD_ERR, CMD_LEN);
    return false;
  }
  
  if (get_network_seed_status()){ // Сеть должна быть отключенна
    cmd_answer_err(ATYPE_CMD_ERR, CMD_SEEDING);
    return true;
  }
  
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  CONFIG.sync_channel = args->CH;
  
  LOG_ON("CMD 0x0E. Set SYNC CH");
  cmd_answer(ATYPE_CMD_OK, NULL, 0);
  return true;
}
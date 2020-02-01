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
@brief Установить системный канал. Сеть выкл.
Информация о системном канала распространяется в пакете синхронизации
*/
bool cmd_0x0F(uint8_t *cmd, uint8_t size)
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
  CONFIG.sys_channel = args->CH;
  
  LOG_ON("CMD 0x0А. Set SYS CH");
  cmd_answer(ATYPE_CMD_OK, NULL, 0);
  return true;
}
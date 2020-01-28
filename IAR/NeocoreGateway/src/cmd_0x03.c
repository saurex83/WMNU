#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "nwdebuger.h"

#define ARGS_SIZE sizeof(cmd_args_s)
typedef struct //!< Аргументы команды
{
  bool on_off;
  uint16_t crc16;
} cmd_args_s;

/**
@brief Включить или выключить сеть. true - вкл
*/
bool cmd_0x03(uint8_t *cmd, uint8_t size)
{
  
  if (size != ARGS_SIZE){ // Размер аргументов не верен
    cmd_answer_err(ATYPE_CMD_ERR, CMD_LEN);
    return false;
  }
  
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  
  bool res;
  if (args->on_off){ // Просят включить сеть
    res = neocore_stack_reset();
    ASSERT(res == true);
    network_seed_enable(true);
    LOG_ON("CMD 0x03. Start network");
  }
  else
  {
    res = neocore_stack_reset();
    ASSERT(res == true);
    LOG_ON("CMD 0x03. Stop network");
  }
  
  cmd_answer(ATYPE_CMD_OK, NULL, 0);
  return true;
}
#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "RADIO.h"
#include "nwdebuger.h"

#define ARGS_SIZE sizeof(cmd_args_s)
typedef struct //!< Аргументы команды
{
  uint8_t FCH;
  uint16_t MS;
  uint16_t crc16;
} cmd_args_s;

/**
@brief Скан канала CH в течении time_ms. Сеть выкл.
@detail Ответ после завершения сканирования
*/
bool cmd_0x0D(uint8_t *cmd, uint8_t size)
{
  if (size != ARGS_SIZE){ // Размер аргументов не верен
    cmd_answer_err(ATYPE_CMD_ERR, CMD_LEN);
    return false;
  }
  
  if (get_network_seed_status()){ // Сеть активна сканировать нельзя
    cmd_answer_err(ATYPE_CMD_ERR, CMD_SEEDING);
    return false;
  }
  
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  
  int8_t rssi;
  bool res = RI_Measure_POW(args->FCH, args->MS, &rssi);
  
  LOG_ON("CMD 0x0D. RSSI scan");
  
  cmd_answer(ATYPE_CMD_OK, (uint8_t*)&rssi, sizeof(rssi));
  return true;
}
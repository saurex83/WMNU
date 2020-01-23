#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "nwdebuger.h"

#define ARGS_SIZE sizeof(cmd_args_s)

static void answ(uint8_t ans);

typedef struct //!< Аргументы команды
{
  uint16_t panid;
  uint16_t crc16;
} cmd_args_s;

enum {no_err = 0, err_seeding = 1};
/**
@brief Установить panid
*/
bool cmd_0x01(uint8_t *cmd, uint8_t size)
{
  
  if (size != ARGS_SIZE) // Размер аргументов не верен
    return false;
  
  if (get_network_seed_status()){ // Сеть активна и менять ничего нельзя
    answ(err_seeding);
    return true;
  }
  
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  
  CONFIG.panid =args->panid;
  LOG_ON("CMD 0x01. Set panid");
  answ(no_err);
  return true;
}

static void answ(uint8_t ans){
  uint8_t an[2];
  an[0] = 0x01; // Результат команды
  an[1] = ans;
  stream_write(an, sizeof(an));  
}
#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "string.h"

#define ARGS_SIZE sizeof(cmd_args_s)

static void answ(uint8_t ans);

typedef struct //!< Аргументы команды
{
  uint8_t KEY[16];
  uint16_t crc16;
} cmd_args_s;

enum {no_err = 0, err_seeding = 1};

/**
@brief Загрузка KEY
*/
bool cmd_0x05(uint8_t *cmd, uint8_t size)
{
  
  if (size != ARGS_SIZE) // Размер аргументов не верен
    return false;

  if (get_network_seed_status()){ // Сеть активна и менять ничего нельзя
    answ(err_seeding);
    return true;
  }
   
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  memcpy(CONFIG.stream_key,  args->KEY, 16);   
 
  answ(no_err);
  return true;
}

static void answ(uint8_t ans){
  uint8_t an[2];
  an[0] = 0x01; // Результат команды
  an[1] = ans;
  stream_write(an, sizeof(an));  
}
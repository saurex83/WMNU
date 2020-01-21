#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "TIC.h"

#define ARGS_SIZE sizeof(cmd_args_s)

static void answ(uint8_t ans);

typedef struct //!< Аргументы команды
{
  uint8_t dont_use;
  uint16_t crc16;
} cmd_args_s;

enum {no_err = 0, err_seeding = 1};

/**
@brief перезарузка
*/
bool cmd_0x06(uint8_t *cmd, uint8_t size)
{
  
  if (size != ARGS_SIZE) // Размер аргументов не верен
    return false;
   
  answ(no_err);
  
  //TODO АППАПРАТНАЯ перезагрузка.
  
  return true;
}

static void answ(uint8_t ans){
  uint8_t an[2];
  an[0] = 0x01; // Результат команды
  an[1] = ans;
  stream_write(an, sizeof(an));  
}
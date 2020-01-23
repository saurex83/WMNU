#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "MAC.h"
#include "nwdebuger.h"

#define ARGS_SIZE sizeof(cmd_args_s)

static void answ(uint8_t ans);

typedef struct //!< Аргументы команды
{
  uint16_t crc16;
} cmd_args_s;


/**
@brief Количество принятых пакетов в буфере RX. Сеть вкл.
*/
bool cmd_0x09(uint8_t *cmd, uint8_t size)
{
  
  if (size != ARGS_SIZE) // Размер аргументов не верен
    return false;
  
 // cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  
  answ(36);
  
  LOG_ON("CMD 0x09. RX frame count");
  return true;
}

static void answ(uint8_t ans){
  uint8_t an[2];
  an[0] = 0x01; // Результат команды
  an[1] = ans;
  stream_write(an, sizeof(an));  
}
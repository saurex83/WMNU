#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"

#define ARGS_SIZE sizeof(cmd_args_s)

static void answ(uint8_t ans);

typedef struct //!< Аргументы команды
{
  bool on_off;
  uint16_t crc16;
} cmd_args_s;

enum {no_err = 0, err_seeding = 1};

/**
@brief Включить или выключить сеть. true - вкл
*/
bool cmd_0x03(uint8_t *cmd, uint8_t size)
{
  
  if (size != ARGS_SIZE) // Размер аргументов не верен
    return false;
    
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  
  if (args->on_off){ // Просят включить сеть
  // TODO Включение сети и рестарт с уничтожением пакетов
  }
  else
  {
    // TODO отключение с уничтожением пакетов
  }
  
  answ(no_err);
  return true;
}

static void answ(uint8_t ans){
  uint8_t an[2];
  an[0] = 0x01; // Результат команды
  an[1] = ans;
  stream_write(an, sizeof(an));  
}
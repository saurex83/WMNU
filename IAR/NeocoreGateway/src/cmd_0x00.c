#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"

enum {no_seed = 0, seeding = 1};
enum {no_err = 0, err_seeding = 1};

static void answ(uint8_t err);

/**
@brief Статус приемника
*/
bool cmd_0x00(uint8_t *cmd, uint8_t size)
{
  if (get_network_seed_status())
    answ(seeding);
  else
    answ(no_seed);
    
  return true;
}

static void answ(uint8_t ans){
  uint8_t an[2];
  an[0] = 0x01; // Результат команды
  an[1] = ans;
  stream_write(an, sizeof(an));  
}
#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "RADIO.h"
#include "nwdebuger.h"

#define ARGS_SIZE sizeof(cmd_args_s)

static void answ(uint8_t ans);
static void answ_data(int8_t rssi);

typedef struct //!< Аргументы команды
{
  uint8_t FCH;
  uint16_t MS;
  uint16_t crc16;
} cmd_args_s;

enum {no_err = 0, err_seeding = 1};

/**
@brief Скан канала CH в течении time_ms. Сеть выкл.
@detail Ответ после завершения сканирования
*/
bool cmd_0x0D(uint8_t *cmd, uint8_t size)
{
  
  if (size != ARGS_SIZE) // Размер аргументов не верен
    return false;
  
  if (get_network_seed_status()){ // Сеть активна сканировать нельзя
    answ(err_seeding);
    return true;
  }
  
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  
  int8_t rssi;
  bool res = RI_Measure_POW(args->FCH, args->MS, &rssi);
  
  LOG_ON("CMD 0x0D. RSSI scan");
  answ_data(rssi);
  
  return true;
}

static void answ(uint8_t ans){
  uint8_t an[2];
  an[0] = 0x01; // Результат команды
  an[1] = ans;
  stream_write(an, sizeof(an));  
}

static void answ_data(int8_t rssi){
  uint8_t an[3];
  an[0] = 0x01; // Результат команды
  an[1] = no_err;  
  an[2] = rssi;
  stream_write(an, sizeof(an));
}
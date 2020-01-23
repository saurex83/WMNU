#include "stdint.h"
#include "cmd_parser.h"
#include "manager.h"
#include "config.h"
#include "MAC.h"
#include "nwdebuger.h"
#include "frame.h"
#include "mem.h"
#include "LLC.h"

#define ARGS_SIZE sizeof(cmd_args_s)

static void answ(uint8_t ans);

typedef struct //!< Аргументы команды
{
  meta_s meta;
  uint8_t len;
  uint8_t offset_payload;
} cmd_args_s;

enum {no_err = 0, err_no_seeding = 1};

/**
@brief Поставить пакет в очередь на отправку TX LLC. Сеть вкл.
*/
bool cmd_0x0B(uint8_t *cmd, uint8_t size)
{
  // Длинна sz{meta} + sz{len} + len(payload) + 2(CRC16)
  if (size > ARGS_SIZE ) // Прежде чем извлеч проверим мин. размер
    return false;
  
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы
  
  if (size != ARGS_SIZE + args->len) // Проверим точный размер
    return false;
  
  if (!get_network_seed_status()){ // Если сеть отключена ничего отпр. ненадо
    answ(err_no_seeding);
    return true;
  }
  
  frame_s* tx_frame = frame_create(); // Создаем фрейм
  re_memcpy(&tx_frame->meta, &args->meta, sizeof(meta_s)); //Копируем метаданные
  
  uint8_t *ptr = &args->offset_payload; // Начало содержимого
  frame_addHeader(tx_frame, ptr, args->len);
  
  // Ставим в очередь на отправку
  bool res = LLC_AddTask(tx_frame);
  
  LOG_ON("CMD 0x0B. Frame to TX");
  answ(no_err);
  return true;
}

static void answ(uint8_t ans){
  uint8_t an[2];
  an[0] = 0x01; // Результат команды
  an[1] = ans;
  stream_write(an, sizeof(an));  
}
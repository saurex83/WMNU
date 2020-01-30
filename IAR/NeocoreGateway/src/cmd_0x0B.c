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
typedef struct //!< Аргументы команды
{
  meta_s meta;
  uint8_t len;
  uint8_t offset_payload; //!< это указатель на первый байт payload
} cmd_args_s;

/**
@brief Поставить пакет в очередь на отправку TX LLC. Сеть вкл.
*/
bool cmd_0x0B(uint8_t *cmd, uint8_t size)
{
  // Длинна sz{meta} + sz{len} + len(payload) + 2(CRC16)
  if (size - 2 < ARGS_SIZE ){ // Прежде чем извлеч проверим мин. размер
    cmd_answer_err(ATYPE_CMD_ERR, CMD_LEN);
    return false;
  }
  
  cmd_args_s *args = (cmd_args_s*)cmd; // Извлекаем аргументы

  if (size - 2 != ARGS_SIZE + args->len - 1){ // Проверим точный размер
    cmd_answer_err(ATYPE_CMD_ERR, CMD_LEN);
    return false;
  }
    
  if (!get_network_seed_status()){ // Если отключена. Ошибка
    cmd_answer_err(ATYPE_CMD_ERR, CMD_NOSEEDING);
    return true;
  }
  
  // TODO проверить значения аргументов
  //cmd_answer_err(ATYPE_CMD_ERR, CMD_ARG_VAL); 
  
  frame_s* tx_frame = frame_create(); // Создаем фрейм
  re_memcpy(&tx_frame->meta, &args->meta, sizeof(meta_s)); //Копируем метаданные
  
  uint8_t *ptr = &args->offset_payload; // Начало содержимого
  frame_addHeader(tx_frame, ptr, args->len);
  
  // Ставим в очередь на отправку
  bool res = LLC_AddTask(tx_frame);
  
  if (!res){ // Буфер полон 
    frame_delete(tx_frame);
    cmd_answer_err(ATYPE_CMD_ERR, CMD_TX_FULL);
    LOG_ON("CMD 0x0B. TX buff full");
    return false;
  }
  
  LOG_ON("CMD 0x0B. Frame add to LLC");
  cmd_answer(ATYPE_CMD_OK, NULL, 0);
  return true;
}
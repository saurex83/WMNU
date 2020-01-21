#include "uart_comm.h"
#include "stdbool.h"
#include "cmd_index_list.h"
#include "nwdebuger.h"

enum {answ_crc_err = 1, answ_args_err = 2, answ_no_cmd = 3}; //!< Коды ошибок

//#define USE_CRC16_CHECK //!< Использовать проверку CRC
#define MAX_CMD_INDEX sizeof(CMD_LIST) //!< Все номера команд меньше этого числа

static unsigned short crc16(unsigned char *pcBlock, unsigned short len);
static bool check_cmd_frame(uint8_t *cmd, uint8_t size);
static void cmd_answ(uint8_t err_code);

typedef bool(*cmd_handler)(uint8_t *cmd, uint8_t size);

// Список обработчиков. Номер команды = индекс в массиве
static cmd_handler CMD_LIST[] = { 
  cmd_0x00, cmd_0x01, cmd_0x02, cmd_0x03, cmd_0x04, cmd_0x05, cmd_0x06,
  cmd_0x07, cmd_0x08, cmd_0x09, cmd_0x0A, cmd_0x0B, cmd_0x0C, cmd_0x0D,
  cmd_0x0E
};    
 


/**
@brief Разбор входящих команд от PC
@detail После приема следует ответ если есть ошибки CRC, args, len.
 Если ошибок нет, то ответ формирует команда.
*/
void parse_uart_stream(void)
{
  uint8_t* cmd;
  uint8_t size;
  uint8_t iCmd;
  
  while (true){
    cmd = uart_recv_cmd(&size);
    if (!check_cmd_frame(cmd,size)){ // Кадр плохой
      cmd_answ(answ_crc_err);
      continue; 
    }
      
    iCmd = cmd[0];
    if (!(iCmd < MAX_CMD_INDEX)){ // Нет такой команды
      cmd_answ(answ_no_cmd);
      continue; 
    }
    
    if (!CMD_LIST[iCmd](cmd, size)){ // Аргум. неверны.
      cmd_answ(answ_args_err);
      continue;
    }
    // Сюда приходим если нет проблем с командой
  }
}

/**
@brief Ответ на команду с ошибкой
@param[in] err_code код ошибки
*/
static void cmd_answ(uint8_t err_code){
  uint8_t an[2];
  an[0] = 0x00;  // Обозначет ответ парсера
  an[1] = err_code;
  uart_write((char*)an, sizeof(an));
}

/**
@brief Отправляем данные в PC. 
@brief Формирует байт размера и добавляет CRC16
*/
void stream_write(uint8_t *data, uint8_t size)
{
  ASSERT(size > 0 && size < 253) // +2 CRC +1 LEN +252 DATA = 255 максимум
  uint8_t nsize = size + 2; // Два байта CRC
  unsigned short data_crc16 = crc16(data, size);
  uart_write((char*)&nsize, 1);
  uart_write((char*)data, size);
  uart_write((char*)&data_crc16 , 2);
}

/**
@brief Проверка коректности принятого пакета
*/
static bool check_cmd_frame(uint8_t *cmd, uint8_t size)
{  
  if (size < 3) // Размер данных не может быть меньше 3 байт
      return false;
    
#ifdef USE_CRC16_CHECK    
  unsigned short* cmd_crc16;
  unsigned short data_crc16;  
  // Извлекаем CRC16 из пакета (последнии 2 байта) 
  cmd_crc16 = (unsigned short*)&cmd[size - 2];
  // Расчитываем CRC16 данных
  data_crc16 = crc16(cmd, size - 2);
  // Если данные битые то продолжаем прием
  if (data_crc16 != *cmd_crc16)
    return false;
#endif

  return true;
}

/**
@brief Расчет CRC16
@param[in] pcBlock указатель на начало масива
@param[in] len размер данных для подсчета
@return код crc16
*/
static unsigned short crc16(unsigned char *pcBlock, unsigned short len){
    unsigned short crc = 0xFFFF;
    unsigned char i;
 
    while (len--)
    {
        crc ^= *pcBlock++ << 8;
 
        for (i = 0; i < 8; i++)
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
 
    return crc;
}

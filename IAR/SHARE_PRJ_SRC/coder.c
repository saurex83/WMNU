/*!
\file 
\brief Функции кодирования данных
*/

#include "coder.h"
#include "Radio_defs.h"

#define AES_CORE(x) {ENCCS = x;}
#define AES_READY  (ENCCS & 8)
#define CBC_MODE  (0x00 << 4)
#define CFB_MODE  (0x01 << 4)
#define OFB_MODE  (0x02 << 4)
#define CTR_MODE  (0x03 << 4)
#define ECB_MODE  (0x04 << 4)
#define CBC_MAC   (0x05 << 4)
#define CMD_ENC   (0x00 << 1)
#define CMD_DCR   (0x01 << 1)
#define CMD_KEY   (0x02 << 1)
#define CMD_IV    (0x03 << 1)
#define CMD_ST    (0x01 << 0)

void AES_init(void)
{
}

void AES_ECB_Encrypt(uint8_t *src, uint8_t *key, uint8_t len)
{
  // Загружаем ключ
  AES_CORE(CMD_ST | ECB_MODE | CMD_KEY );
  for (uint8_t i = 0; i < 16; i++)
    ENCDI = key[i];

  // Загружаем IV
  uint8_t r= 43;
  AES_CORE(CMD_ST | ECB_MODE | CMD_IV );
  for (uint8_t i = 0; i < 16; i++)
  {
    ENCDI = r;
    r++;
  }
  
  uint8_t *rd_src = src, *wr_src = src;
  uint8_t rd_len = len, wr_len = len;

  // Шифруем данные
  while (rd_len > 0 )
  {
    // Начинаем кодировать блок 128 бит 
    AES_CORE(CMD_ST| ECB_MODE | CMD_ENC);
   
    for (uint8_t i = 0; i < 16; i++)
    {
      if (rd_len) // Если данные кончились, отсылаем нули
      {
        ENCDI = *rd_src;
        rd_src++;
        rd_len -- ;
      }
      else
        ENCDI = 0x00;
    }
    
    // Ждем завершения шифрования блока
   // while (!AES_READY);
    
    // Выгружаем шифрованый блок
    for (uint8_t i = 0; i < 16; i++)
    {
      if (wr_len)
      {
        *wr_src = ENCDO;
        wr_src ++;
        wr_len --;
      }
      else
        break;
    }
  }
}
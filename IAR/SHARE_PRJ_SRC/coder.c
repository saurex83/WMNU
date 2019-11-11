/*!
\file 
\brief Функции кодирования данных
*/

#include "coder.h"
#include "Radio_defs.h"
#include "string.h"
#include "delays.h" // Для профелирования, Отладка

#define AES_START()  {ENCCS |= 0x01;} //!< Запуск выполнения команды  
#define AES_SET_MODE(mode) {ENCCS &= ~0x70; ENCCS |= mode;} //!< Установка режима
#define AES_SET_OPERATION(op) {ENCCS = (ENCCS & ~0x07) | op;} 
#define AES_RDY() {ENCCS & 8)} //!< Состояние модуля

#define ST_DEF(STRUCT, FILD, VAL)  STRUCT.FILD = VAL
#define HADDR(ADDR) ((uint16_t)ADDR >> 8)
#define LADDR(ADDR) ((uint16_t)ADDR)

// Режимы шифрования
#define AES_MODE_CBC            0x00
#define AES_MODE_CFB            0x10
#define AES_MODE_OFB            0x20
#define AES_MODE_CTR            0x30
#define AES_MODE_ECB            0x40
#define AES_MODE_CBCMAC         0x50

// Операции 
#define AES_ENCRYPT             0x00
#define AES_DECRYPT             0x02
#define AES_LOAD_KEY            0x04
#define AES_LOAD_IV             0x06

#define ENC_DW 29 // DMA AES тригер запрос загрузки
#define ENC_UP 30 // DMA AES тригер запрос выгнрузки

#define STREAM_ENC_MODE     AES_MODE_OFB //!< Метод шифрования потока данных

// Доступные методы
void AES_StreamCoder(bool enc_mode, uint8_t *src, uint8_t *dst, 
                     uint8_t *key, uint8_t *nonce, uint8_t len);

// Приватные функции

typedef struct //!< Структура блока B0 для режима CCM
{
  struct 
  {
    uint8_t L:3;
    uint8_t M:3;
    uint8_t A_Data:1;
  } flag;
  uint16_t nonce[9];
  uint8_t L_M[6];
} __attribute__((packed)) B0_s;

typedef struct //!< Структура блока A0 для режима CCM
{
  struct 
  {
    uint8_t L:3;
  } flag;
  uint8_t nonce[11];
  uint8_t ctr;
} __attribute__((packed)) A0_s;

typedef struct //!< Структура с настройками DMA 
{
  uint8_t  SRCADDRH;
  uint8_t  SRCADDRL;
  uint8_t  DSTADDRH;
  uint8_t  DSTADDRL;
  struct  {
  uint8_t   LENH        :5;
  uint8_t   VLEN        :3;
  };
  struct  {
  uint8_t   LENL        :8;
  };
  struct  {
  uint8_t   TRIG        :5;
  uint8_t   TMODE       :2;
  uint8_t   WORDSIZE    :1;
  };
  struct {
  uint8_t   PRIORITY    :2;
  uint8_t   M8          :1;
  uint8_t   IRQMASK     :1;
  uint8_t   DESTINC     :2;
  uint8_t   SRCINC      :2;
  };
} __attribute__((packed)) DMA_AES_s ;

DMA_AES_s DMA_AES_DW; //!< DMA на запись
DMA_AES_s DMA_AES_UP; //!< DMA на чтение

void AES_init(void)
{
  // Настроим канал 0 DMA для загрузки данных в AES
  uint16_t CONF_ADDR = (uint16_t)&DMA_AES_DW;
  DMA0CFGH = CONF_ADDR >> 8;
  DMA0CFGL = CONF_ADDR & 0xFF;
  
  // Настроим канал 1 DMA для выгрузки данных из AES
  CONF_ADDR = (uint16_t)&DMA_AES_UP;
  DMA1CFGH = CONF_ADDR >> 8;
  DMA1CFGL = CONF_ADDR & 0xFF;
  
  ST_DEF(DMA_AES_DW, DSTADDRH, 0x70); // Пишем данные XENCDI = 0xB1 
  ST_DEF(DMA_AES_DW, DSTADDRL, 0xB1); //  
  ST_DEF(DMA_AES_DW, PRIORITY, 0x00); // Низкий приоритет
  ST_DEF(DMA_AES_DW, M8, 0x00); // Используем 8 бит для счетика длинны
  ST_DEF(DMA_AES_DW, IRQMASK, 0x00); // Запрещаем генерировать перывания
  ST_DEF(DMA_AES_DW, DESTINC, 0x00); // Не увеличиваем адресс назначения
  ST_DEF(DMA_AES_DW, SRCINC, 0x01); // Увеличиваем адресс источника
  ST_DEF(DMA_AES_DW, TRIG, ENC_DW); // Тригер по загрузке
  ST_DEF(DMA_AES_DW, WORDSIZE, 0x00); // Копируем по 1 байту
  ST_DEF(DMA_AES_DW, TMODE, 0x01); //  Блочное копирование по тригеру
  ST_DEF(DMA_AES_DW, VLEN, 0x00); //  Количество байт определяет поле LEN  
  ST_DEF(DMA_AES_DW, LENH, 0x00); 
  
  ST_DEF(DMA_AES_UP, SRCADDRH, 0x70); // Читаем данные из X_ENCDO  
  ST_DEF(DMA_AES_UP, SRCADDRL, 0xB2);  
  ST_DEF(DMA_AES_UP, PRIORITY, 0x00); // Низкий приоритет
  ST_DEF(DMA_AES_UP, M8, 0x00); // Используем 8 бит для счетика длинны
  ST_DEF(DMA_AES_UP, IRQMASK, 0x00); // Запрещаем генерировать перывания
  ST_DEF(DMA_AES_UP, DESTINC, 0x01); // Увеличиваем адресс назначения
  ST_DEF(DMA_AES_UP, SRCINC, 0x00); // Не увеличиваем адресс источника
  ST_DEF(DMA_AES_UP, TRIG, ENC_UP); // Тригер по выгрузке
  ST_DEF(DMA_AES_UP, WORDSIZE, 0x00); // Копируем по 1 байту
  ST_DEF(DMA_AES_UP, TMODE, 0x01); //  Блочное копирование по тригеру
  ST_DEF(DMA_AES_UP, VLEN, 0x00); //  Количество байт определяет поле LEN  
  ST_DEF(DMA_AES_UP, LENH, 0x00); 
}

/**
@brief Шифрует/дешифрует поток данных во время радиопередачи. Без проверки MIC
@param[in] enc_mode Режим работы процедуры. true - шифрование.
@param[in] src Указатель на данные подлежащии шифрованию
@param[in] dst Указтель куда будут помещены зашифрованные данные
@param[in] key Указатье на ключ. 16 байт
@param[in] iv  Указатель на вектор иницилизации. 16 байт 
@param[in] len Длинна данных
*/
void AES_StreamCoder(bool enc_mode, uint8_t *src, uint8_t *dst, uint8_t *key,
                         uint8_t *iv, uint8_t len)
{
  // Установим метод кодироваения
  AES_SET_MODE(STREAM_ENC_MODE);
    
  // Загружаем ключ
  AES_SET_OPERATION(AES_LOAD_KEY);
  ST_DEF(DMA_AES_DW, SRCADDRL, LADDR(key));
  ST_DEF(DMA_AES_DW, SRCADDRH, HADDR(key));
  ST_DEF(DMA_AES_DW, LENL, 16);
  DMAARM |= 0x01;
  AES_START();
  while (DMAARM);

  // Загружаем IV
  AES_SET_OPERATION(AES_LOAD_IV);
  ST_DEF(DMA_AES_DW, SRCADDRL, LADDR(iv));
  ST_DEF(DMA_AES_DW, SRCADDRH, HADDR(iv));
  ST_DEF(DMA_AES_DW, LENL, 16);
  DMAARM |= 0x01;
  AES_START();
  while (DMAARM);

  // Установим необходимую операцию AES
  if (enc_mode) 
    AES_SET_OPERATION(AES_ENCRYPT)
  else 
    AES_SET_OPERATION(AES_DECRYPT)
  
  switch(STREAM_ENC_MODE) 
  {
  case AES_MODE_ECB: // ECB, CBC не реализованы
  case AES_MODE_CBC:
    break;
  case AES_MODE_CFB:
  case AES_MODE_OFB:
  case AES_MODE_CTR:
    {
      // Загрузка блоками по 128 бит с разделением на под-блоки по 32 бита
      uint8_t nbrBlocks = len / 16; // Количество целых блоков по 128 бит
      uint8_t ptr, sub_ptr; // Смещение
      uint8_t *download, *upload;
      
      // Для этих типов шифрования длина блоков по 4 байта
      ST_DEF(DMA_AES_DW, LENL, 4);
      ST_DEF(DMA_AES_UP, LENL, 4);
      
      // Шифруем все целые блоки
      for (uint8_t block = 0; block < nbrBlocks; block ++)
      {
        ptr = 16 * block;
        AES_START();
        for (uint8_t j = 0; j < 4; j++)
        {
          sub_ptr = ptr + 4*j;
          download = &src[sub_ptr];
          upload = &dst[sub_ptr];
          // Указываем адресс DMA откуда читать данные
          ST_DEF(DMA_AES_DW, SRCADDRL, LADDR(download));
          ST_DEF(DMA_AES_DW, SRCADDRH, HADDR(download));
          // Указываем адрес DMA куда записывать данные          
          ST_DEF(DMA_AES_UP, DSTADDRL, LADDR(upload));
          ST_DEF(DMA_AES_UP, DSTADDRH, HADDR(upload));
          // Активируем DMA
          DMAARM |= 0x03;  
          DMAREQ |= 0x01;
          while (DMAARM);          
        }
      }     
      // Шифруем последний блок
      uint8_t block_len = len % 16; // Размер последнего блока
      
      // Завершаем работу если блок пустой
      if (!block_len)
        return;
      
      uint8_t padding_block[16]; // Блок заполненый нулями
      ptr = 16*nbrBlocks; // Смещение на первый байт последнего блока в src
      memset(padding_block, 0x00, sizeof(padding_block)); // Заполняем нулями
      memcpy(padding_block, &src[ptr], block_len); // Копируем данные
      
      AES_START();
      for (uint8_t j = 0; j < 4; j++)
        {
          sub_ptr = 4*j;
          download = &padding_block[sub_ptr];
          // Указываем адресс DMA откуда читать данные
          ST_DEF(DMA_AES_DW, SRCADDRL, LADDR(download));
          ST_DEF(DMA_AES_DW, SRCADDRH, HADDR(download));
          // Указываем аддрес DMA куда записывать данные          
          ST_DEF(DMA_AES_UP, DSTADDRL, LADDR(download));
          ST_DEF(DMA_AES_UP, DSTADDRH, HADDR(download));
          // Активируем DMA
          DMAARM |= 0x03;  
          DMAREQ |= 0x01;
          while (DMAARM);
        }
      memcpy(&dst[ptr], padding_block, block_len); // Копируем в src
      
    } //CASE
    
    break;
  } 
}


/**
@brief Шифрует/дешифрует данные с проверкой MIC
@param[in] enc_mode Режим работы процедуры. true - шифрование.
@param[in] src Указатель на данные подлежащии шифрованию
@param[in] dst Указтель куда будут помещены зашифрованные данные
@param[in] key Указатье на ключ. 16 байт
@param[in] iv  Указатель на вектор иницилизации. 16 байт 
@param[in] len Длинна данных
*/
bool AES_DataCoder(bool enc_mode, uint8_t *src, uint8_t *dst, 
                     uint8_t *key, uint8_t *nonce, uint8_t len)
{
  // Установим метод кодироваения
  AES_SET_MODE(STREAM_ENC_MODE);
    
  // Загружаем ключ
  AES_SET_OPERATION(AES_LOAD_KEY);
  ST_DEF(DMA_AES_DW, SRCADDRL, LADDR(key));
  ST_DEF(DMA_AES_DW, SRCADDRH, HADDR(key));
  ST_DEF(DMA_AES_DW, LENL, 16);
  DMAARM |= 0x01;
  AES_START();
  while (DMAARM);
  
  //****************************
  //Message Authentication Phase
  //****************************
  
  // Шаг 1. Загружаем IV нули 
  uint8_t iv_zero[16];
  memset(iv_zero, 0 , sizeof(iv_zero));
  AES_SET_OPERATION(AES_LOAD_IV);
  ST_DEF(DMA_AES_DW, SRCADDRL, LADDR(key));
  ST_DEF(DMA_AES_DW, SRCADDRH, HADDR(key));
  DMAARM |= 0x01;
  AES_START();
  while (DMAARM);    
  
  // Шаг 2. Создаем блок B0
  B0_s B0 = {.flag.L = 2};
  
  return true;
}
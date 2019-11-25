/*!
\file Модуль шифрования данных
\brief Функции кодирования данных
*/

#include "coder.h"
#include "ioCC2530.h"
#include "string.h"
#include "delays.h" // Для профилирования, Отладка

#define AES_START()  {ENCCS |= 0x01;} //!< Запуск выполнения команды  
#define AES_SET_MODE(mode) {ENCCS &= ~0x70; ENCCS |= mode;} //!< Установка режима
#define AES_SET_OPERATION(op) {ENCCS = (ENCCS & ~0x07) | op;} 
#define AES_RDY() (ENCCS & 8) //!< Состояние модуля

#define ST_DEF(STRUCT, FILD, VAL)  STRUCT.FILD = VAL
#define HADDR(ADDR) ((uint16_t)ADDR >> 8)
#define LADDR(ADDR) ((uint16_t)ADDR)
#define BV(n)                   (1 << (n))
#define MIC_2_MICLEN(m)         (BV((m&3)+1) & ~3)

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
#define ENC_UP 30 // DMA AES тригер запрос выгрузки

#define STREAM_ENC_MODE     AES_MODE_OFB //!< Метод шифрования потока данных

// Доступные методы
void AES_StreamCoder(bool enc_mode, uint8_t *src, uint8_t *dst, 
                     uint8_t *key, uint8_t *nonce, uint8_t len);

// Приватные функции
static inline uint8_t generateAuthData(uint8_t *src, uint8_t *nonce, uint8_t c,
                                       uint8_t f, uint8_t lm);
static void CBCMAC_buf_encrypt(uint8_t len, uint8_t *key, uint8_t *mac);

/**
@brief Локальный буфер для работы режима CCM
@detail 128 длина сообщения. 18 длинна блока B0 и строки состояния.
 16 длина дополнения нулями
*/
static uint8_t buf[128+18+16]; 


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


/**
@brief Иницилизация модуля
@detail Модуль использует DMA каналы 0 и 1
*/
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
@brief Шифрует/дешифрует поток данных во время радиопередачи. Без проверки MIC
@param[in] enc_mode Режим работы процедуры. true - шифрование.
@param[in] src Указатель на данные подлежащии шифрованию
@param[in] dst Указтель куда будут помещены зашифрованные данные
@param[in] key Указатье на ключ. 16 байт
@param[in] iv  Указатель на вектор иницилизации. 16 байт 
@param[in] len Длинна данных
*/
static void CTR_enc_decrypt(bool enc_mode, uint8_t *src, uint8_t *dst, uint8_t *key,
                         uint8_t *iv, uint8_t len)
{
  // Установим метод кодироваения
  AES_SET_MODE(AES_MODE_CTR);
  
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
}

/**
@brief Зашифровывает buf в режиме CBC-MAC с IV = 0
@param[in] len улинна последовательности для вычисления MAC
@param[out] mac указатель на память куда будет записан mac (до 16 байт)
*/
static void CBCMAC_buf_encrypt(uint8_t len, uint8_t *key, uint8_t *mac)
{
  uint8_t IV[16];
  
  // Заполняем вектор нулями
  memset(IV, 0x00, sizeof(IV));
  
  // Установим метод кодироваения
  AES_SET_MODE(AES_MODE_CBCMAC);  
  
  { // Сворачиваем код для улучшения чтения
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
  ST_DEF(DMA_AES_DW, SRCADDRL, LADDR(IV));
  ST_DEF(DMA_AES_DW, SRCADDRH, HADDR(IV));
  ST_DEF(DMA_AES_DW, LENL, 16);
  DMAARM |= 0x01;
  AES_START();
  while (DMAARM);
  };
  
  // Устанавливаем операцию шифрования
  AES_SET_OPERATION(AES_ENCRYPT);
  
  // Загрузка блоками по 128 бит
  uint8_t nbrBlocks = len / 16; // Количество целых блоков по 128 бит
  uint8_t block_len = len % 16; // Размер последнего блока
  uint8_t ptr; // Смещение
      
  // Для этого типа шифрования длина блоков по 16 байт
  ST_DEF(DMA_AES_DW, LENL, 16);
  // Устанавливаем куда будем выгружать вычисленный MAC
  ST_DEF(DMA_AES_UP, DSTADDRL, LADDR(mac));
  ST_DEF(DMA_AES_UP, DSTADDRH, HADDR(mac));
  ST_DEF(DMA_AES_UP, LENL, 16);
  
  // Шифруем все целые блоки
  for (uint8_t block = 0; block < nbrBlocks; block ++)
    {
      // Последний блок шифруем в режиме CBC при условии что нет блок не 
      // кратного 16 байтам.
      if (!block_len && (block == nbrBlocks - 1))
        AES_SET_MODE(AES_MODE_CBC);
          
      ptr = 16 * block;
      while(!AES_RDY());
      AES_START();
      // Указываем адресс DMA откуда читать данные
      ST_DEF(DMA_AES_DW, SRCADDRL, LADDR(buf[ptr]));
      ST_DEF(DMA_AES_DW, SRCADDRH, HADDR(buf[ptr]));
      // Активируем DMA
      DMAARM |= 0x01;  
      // Активируем выгрузку только последнего блока
      if (!block_len && (block == nbrBlocks - 1))
        DMAARM = 0x03;
      
      DMAREQ |= 0x01;
      while (DMAARM);          
     }
       
  // Шифруем последний блок в режиме CBC
  AES_SET_MODE(AES_MODE_CBC);
          
  // Завершаем работу если блок пустой
  if (!block_len)
    return;
      
  uint8_t padding_block[16]; // Блок заполненый нулями
  ptr = 16*nbrBlocks; // Смещение на первый байт последнего блока в src
  memset(padding_block, 0x00, sizeof(padding_block)); // Заполняем нулями
  memcpy(padding_block, &buf[ptr], block_len); // Копируем данные
      
  while(!AES_RDY());
  AES_START();
  // Указываем адресс DMA откуда читать данные
  ST_DEF(DMA_AES_DW, SRCADDRL, LADDR(padding_block));
  ST_DEF(DMA_AES_DW, SRCADDRH, HADDR(padding_block));
  // Активируем DMA и выгрузку MAC
  DMAARM |= 0x03;  
  DMAREQ |= 0x01;
  while (DMAARM);
}

static inline uint8_t generateAuthData(uint8_t *src, uint8_t *nonce, uint8_t c,
                                       uint8_t f, uint8_t lm)
{
  memcpy(buf, nonce,16);
  
  // Буфер от 0 до 15 байта специальный блок B0
  // Настраиваем флаг. У нас длинна 2 байта. nonce 13 байт => L_M = L-1=0x01
  buf[0]=  0x01;  
  // Если есть данные для авторизации установим A_Data 
  if (f > 0)
    buf[0] |= 0x40;
  
  buf[0] |= ((lm - 2) / 2 ) << 3;   // см. документацию M'= (lm-2) / 2;
  
  // Устанавливаем длинну сообщения
  buf[14] = 0x00;
  buf[15] = c;
  
  // Добавляем строку авторизации L(a). если данных нет, то она пустая. f=0
  buf[16]= 0;
  buf[17]= f;
  
  // Копируем данные авторизации в буфер
  memcpy(&buf[18], src, f);
  
  // Смещение на следующий байт после данных авторизации.
  // Данные авторизации занимают положение в буфере [18, 18+f]
  uint8_t ptr_end= 18 + f;
  // Заполняем нулями до границы 16 байт  
  while (ptr_end & 0x0f)
    buf[ptr_end++] = 0x00;
  
  // Копируем данные для шифрования в буфер после нулей
  memcpy(&buf[ptr_end], &src[f], c);
  
  // Возвращаем размер данных в буфере
  return ptr_end+c;
};

/**
@brief Шифрует/дешифрует данные с проверкой MIC
@param[in] src Указатель на данные подлежащии шифрованию
@param[in] len Размер данных
@param[in] с Количество байт для шифрования
@param[in] f Количество байт для авторизации
@param[in] m Размер MIC (m=1,2,3 l(m) = 4,8,16 байт)
@param[in] iv  Указатель на вектор иницилизации. 16 байт 
@param[in] len Длинна данных
@param[in,out] MIC Указатель на массив соответсвующий длинне (4,8,16 байт)
*/
void AES_CCMEncrypt( uint8_t *src, uint8_t c, uint8_t f, uint8_t m,
                    uint8_t *key, uint8_t *nonce, uint8_t *MIC)
{
  uint8_t lm = MIC_2_MICLEN(m);
  
  // Генерируем данные для авторизации
  uint8_t dlen = generateAuthData(src, nonce, c, f, lm);
  
  uint8_t mac[16]; // Сюда пишем мак
  CBCMAC_buf_encrypt(dlen, key, mac);
  
  // Шифруем MAC
  uint8_t CTR_IV[16];
  memcpy(CTR_IV, nonce, 16);
  CTR_IV[0] = 0x01; // Флаг режима CTR
  CTR_IV[15] = 0;   // Значение счетчика
  CTR_enc_decrypt(true, mac, mac, key, CTR_IV, 16);
  
  // Шифруем текст. он имеет размер 'c' и находится в буфере начиная с
  // dlen-c, dlen
  uint8_t ds = dlen - c; // Начало данных для шифрования
  
  // 0x0F & (16 - (dlen & 0x0f)) оставляет числа от 0 до 15  
  memset(buf+dlen, 0x00, 0x0F & (16 - (dlen & 0x0f)) ); // Дополняем нулями
  CTR_IV[15] = 1; // Счетчик CTR
  CTR_enc_decrypt(true, &buf[ds], &buf[ds], key, CTR_IV, c); // Шифруем
  
  // Заполняем буфер буфер src. src[0,f] это данные авторизации. 
  // src[f, f+c] это шифрованные данные.
  memcpy(&src[f], &buf[ds], c); 
  memcpy(MIC, mac, lm);
  
  //https://github.com/zhaohengyi/CC2530Example/blob/
  //master/source/components/radios/cc2530/hal_rf_security.c
}

/**
@brief Дешифрует данные с проверкой MIC
@param[in] src Указатель на данные подлежащии шифрованию
@param[in] len Размер данных
@param[in] с Количество байт для шифрования
@param[in] f Количество байт для авторизации
@param[in] m Размер MIC (m=1,2,3 l(m) = 4,8,16 байт)
@param[in] iv  Указатель на вектор иницилизации. 16 байт 
@param[in] len Длинна данных
@param[in,out] MIC Указатель на массив соответсвующий длинне (4,8,16 байт)
*/
bool AES_CCMDecrypt( uint8_t *src, uint8_t c, uint8_t f, uint8_t m,
                    uint8_t *key, uint8_t *nonce, uint8_t *MIC)
{
  uint8_t lm = MIC_2_MICLEN(m);
  
  // Расшифровываем MAC
  uint8_t mac[16]; // Расшифрованый мак
  uint8_t CTR_IV[16];
  memcpy(CTR_IV, nonce, 16);
  CTR_IV[0] = 0x01; // Флаг режима CTR
  CTR_IV[15] = 0;   // Значение счетчика
  CTR_enc_decrypt(false, MIC, mac, key, CTR_IV, lm);  
  
  // Генерируем данные для авторизации
  uint8_t dlen = generateAuthData(src, nonce, c, f, lm);
  
  // Дешифруем текст. он имеет размер 'c' и находится в буфере начиная с
  // dlen-c, dlen
  uint8_t ds = dlen - c; // Начало данных для дешифрования
  
  // 0x0F & (16 - (dlen & 0x0f)) оставляет числа от 0 до 15  
  memset(buf+dlen, 0x00, 0x0F & (16 - (dlen & 0x0f)) ); // Дополняем нулями
  CTR_IV[15] = 1; // Счетчик CTR
  CTR_enc_decrypt(false, &buf[ds], &buf[ds], key, CTR_IV, c); // Дешифруем
  
  uint8_t new_mac[16]; // мак полученный в ходе рашифровки сообщения
  CBCMAC_buf_encrypt(dlen, key, new_mac);
  
  // Проверяем что маки совпадают, значит дешифровано верно
  bool mac_equal = true;
  for (uint8_t i = 0 ; i < lm ; i++)
  {
    if (mac[i] != new_mac[i])
    mac_equal = false;
    break;
  }
  
  // Выходим если MACи не совпали  
  if (!mac_equal)
    return false;
  
  // Копируем  расшифрованные данные
  memcpy(&src[f], &buf[ds], c); 
  return true;
}
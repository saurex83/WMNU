#include "nwdebuger.h"
#include "ioCC2530.h"
#include "stdint.h"
#include "DMA.h"

typedef struct //!< Структура с настройками DMA для UART 
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
} __attribute__((packed)) DMA_UART_s;

#define UART_RX_BUF_LEN 257 //!< Размер приемного буфера
DMA_UART_s DMA_UART_DR; //!< DMA на чтение данных
static uint8_t uart_rx_buff[UART_RX_BUF_LEN];

static void uart_dma_init(void);

static void uart_dma_init(void)
{
  // Настроим канал 2 DMA для загрузки данных из uart
  // DMA0 и 1 заняты кодером AES
  
  ST_DEF(DMA_CH[2], SRCADDRL, 0x70); //U0DBUF
  ST_DEF(DMA_CH[2], SRCADDRH, 0xC1);
  ST_DEF(DMA_CH[2], LENL, 16);
  ST_DEF(DMA_CH[2], DSTADDRH, LADDR(uart_rx_buff)); // Пишем данные XENCDI = 0xB1 
  ST_DEF(DMA_CH[2], DSTADDRL, HADDR(uart_rx_buff)); //  
  ST_DEF(DMA_CH[2], PRIORITY, 0x00); // Низкий приоритет
  ST_DEF(DMA_CH[2], M8, 0x00); // Используем 8 бит для счетика длинны
  ST_DEF(DMA_CH[2], IRQMASK, 0x00); // Запрещаем генерировать перывания
  ST_DEF(DMA_CH[2], DESTINC, 0x01); // Увеличиваем адресс назначения
  ST_DEF(DMA_CH[2], SRCINC, 0x00); // Не увеличиваем адресс источника
  ST_DEF(DMA_CH[2], TRIG, 0); // Тригер по загрузке
  ST_DEF(DMA_CH[2], WORDSIZE, 0x00); // Копируем по 1 байту
  ST_DEF(DMA_CH[2], TMODE, 0x01); //  Блочное копирование по тригеру
  ST_DEF(DMA_CH[2], VLEN, 0x00); //  Количество байт определяет поле LEN  
  ST_DEF(DMA_CH[2], LENH, 0x00); 
}

/**
@brief Иницилизация uart с компьютером
@detail Скорость 2М бод. Частота микроконтроллера 32Мгц.
*/
void com_uart_init(void)
{
  // Выбор расположения UART на выводах
  uint8_t U0CFG = 0; // 0={TX=P0_3, RX=P0_2
  PERCFG |= (U0CFG<<0); 
  
  // U0UCR регистр настройки режима uart. меня устраивает по умолчанию
  U0CSR = (1<<7); // Выбираем режим uart  
  
  // Настройка скорости передачи данных на 2М  
  U0BAUD = 0;  // табличные значения из pdf
  U0GCR =  16;
  
  // Включаем альтернативные функции выводов
  P0SEL = (1<<2)|(1<<3);
  U0CSR |= (1<<6);
//  uart_dma_init();
}

void uart_putchar(char x){
  while( U0CSR&(1<<0));
  U0DBUF = x;
}

char uart_getchar(void)
{
  char x;
  while( !(U0CSR&(1<<2)));
  x = U0DBUF;
  return x;
}

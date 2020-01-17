#include "utest_suite.h"
#include "manager.h"
#include "nwdebuger.h"
#include "ioCC2530.h"

// Собирать проект с ключем GATEWAY в файле basic.h
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



void suite_GW(void)
{
  bool con = MG_Connect();
  com_uart_init();
  char x;
  
  if (con)
  {
    LOG_ON("Network created\r\n");
  }  
  else
  {
    LOG_ON("Network creation failed\r\n");
  }
  
  uart_putchar('F');
  LOG_ON("Echo\r\n");
  while(1)
  {
    x= uart_getchar();
    uart_putchar(x);  
  };
}
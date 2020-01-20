#include "manager.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "stdint.h"
#include "uart_comm.h"

// Собирать проект с ключем GATEWAY в файле basic.h
void main(void)
{
  bool con = MG_Connect();
 // bool con = true;
  CLKCONCMD &= ~(1<<7); // Сбрасываем бит OSC32K, будет выбран 32.768 кварц
  while (CLKCONSTA & (1<<7)); // Ждем пока бит не станет 0
  CLKCONCMD = 0;// (1<<3);
  while (CLKCONSTA&(1<<6));
  
  com_uart_init();
  
  char buff[30];
  
  if (con)
  {
    LOG_ON("Network created\r\n");
  }  
  else
  {
    LOG_ON("Network creation failed\r\n");
  }
 
  uint8_t* rcv;
  uint8_t sz;
  uart_putchar('F');
  LOG_ON("Echo\r\n");
  while(1)
  {
    
    rcv = uart_recv_cmd(&sz);
    uart_write((char*)&rcv[1], sz);
  };
}
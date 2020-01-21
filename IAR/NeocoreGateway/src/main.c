#include "manager.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "stdint.h"
#include "uart_comm.h"
#include "cmd_parser.h"

// Собирать проект с ключем GATEWAY в файле basic.h
void main(void)
{
  bool con = MG_Connect();
  
  com_uart_init();
  parse_uart_stream();
}
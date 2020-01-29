#include "manager.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "stdint.h"
#include "uart_comm.h"
#include "cmd_parser.h"
extern  char uart_getchar(void);
// Собирать проект с ключем GATEWAY в файле basic.h
void main(void)
{
  neocore_hw_init();
  network_seed_enable(false);
  com_uart_init();
  parse_uart_stream();
}
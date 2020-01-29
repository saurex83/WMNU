#include "manager.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "stdint.h"
#include "uart_comm.h"
#include "cmd_parser.h"


#ifndef GATEWAY
#warning ENABLE GATEWAY DEFINE
#endif

// Собирать проект с ключем GATEWAY в файле basic.h
void main(void)
{
  neocore_hw_init();
  P1DIR |= 0x01;
  network_seed_enable(false);
  com_uart_init();
  parse_uart_stream();
}
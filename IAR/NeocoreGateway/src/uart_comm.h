#pragma once
#include "stdint.h"

void com_uart_init(void);
uint8_t* uart_recv_cmd(uint8_t *size);
void uart_putchar(char x);
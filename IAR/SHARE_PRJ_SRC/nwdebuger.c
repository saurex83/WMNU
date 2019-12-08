#include "nwdebuger.h"
#include "ioCC2530.h"
#include "stdint.h"
#include "stdio.h"

// Настройка порта uart для отладки
static void uart_init(void)
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
}

static void SetCPU32M(void)
{
  CLKCONCMD = 0x88;
  while (CLKCONSTA&(1<<6));
}

void nwDebugerInit(void)
{
  SetCPU32M();
#ifdef UART_DEBUG
  uart_init();
#endif
}

__attribute__((weak)) void STACK_FAILURE(char* msg)
{
  LOG_ON("STACK FAILURE. HALT");
  while(1);
}

// Переопределяем функцию записи в порт
#ifdef UART_DEBUG
#include <yfuns.h>

_STD_BEGIN

#pragma module_name = "?__write"

int MyLowLevelPutchar(int x);

int MyLowLevelPutchar(int x){
  while( U0CSR&(1<<0));
  U0DBUF = x;
  return x;
}
/*
 * If the __write implementation uses internal buffering, uncomment
 * the following line to ensure that we are called with "buffer" as 0
 * (i.e. flush) when the application terminates.
 */

size_t __write(int handle, const unsigned char * buffer, size_t size)
{
  /* Remove the #if #endif pair to enable the implementation */
   

  size_t nChars = 0;

  if (buffer == 0)
  {
    /*
     * This means that we should flush internal buffers.  Since we
     * don't we just return.  (Remember, "handle" == -1 means that all
     * handles should be flushed.)
     */
    return 0;
  }

  /* This template only writes to "standard out" and "standard err",
   * for all other file handles it returns failure. */
  if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR)
  {
    return _LLIO_ERROR;
  }

  for (/* Empty */; size != 0; --size)
  {
    if (MyLowLevelPutchar(*buffer++) < 0)
    {
      return _LLIO_ERROR;
    }

    ++nChars;
  }

  return nChars;

}

_STD_END
#endif
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "stdint.h"
#include "stdio.h"

// Настройка выводов SIG и порта uart для отладки
void nwDebugerInit(void)
{
  CLKCONCMD = 0x88;
  while (CLKCONSTA&(1<<6));
  
  // Выбор расположения UART на выводах
  uint8_t U0CFG = 0; // 0={TX=P0_3, RX=P0_2
  uint8_t U1CFG = 0; // 0={TX=P0_3, RX=P0_2
  PERCFG |= (U0CFG<<0); 
  PERCFG |= (U1CFG<<1);
  // U0UCR регистр настройки режима uart. меня устраивает по умолчанию
  
  U0CSR = (1<<7); // Выбираем режим uart 
  U1CSR = (1<<7); // Выбираем режим uart
  
//  U0UCR |= (1<<6);
 // U1UCR |= (1<<6);
  // Настройка скорости передачи данных на 115200  
  U0BAUD = 0;  // табличные значения из pdf
  U0GCR =  16;

  U1BAUD = 216;  // табличные значения из pdf
  U1GCR =  11;

  
  P0SEL = 0xff;
  P0DIR = 0xff;
  
  P1SEL = 0xff;
  P1DIR = 0xff;
  
  //U0DBUF - буфер передатчика
  
  while(1)
  {
    printf("TEST\r\n");
  }
  
  while(1)
  {
    U0DBUF = 'A';
    U1DBUF = 'B';
    
    while( U0CSR&(1<<0));
    while( U1CSR&(1<<0));
    
    U0DBUF = '\n';
    U1DBUF = '\n';
    
    while( U0CSR&(1<<0));
    while( U1CSR&(1<<0));
  }
  
  P0DIR = 0xff;
  P0_0 = 0;
  P0_1 = 0;
  P0_1 = 0;
  P0_2 = 0;
  P0_3 = 0;
  P0_4 = 0;
  P0_5 = 0;
  P0_6 = 0;
  P0_7 = 0;
  
  P1DIR = 0xff;
  P1_0 = 0;
  P1_1 = 0;
  P1_1 = 0;
  P1_2 = 0;
  P1_3 = 0;
  P1_4 = 0;
  P1_5 = 0;
  P1_6 = 0;
  P1_7 = 0;
  
  P2DIR = 0xff;
  P2_0 = 0;
  P2_1 = 0;
  P2_1 = 0;
  P2_2 = 0;
  P2_3 = 0;
  P2_4 = 0;
  
}

__attribute__((weak)) void STACK_FAILURE(char* msg)
{
  LOG(MSG_ON | MSG_ALARM | MSG_ALL, "STACK FAILURE. HALT\n");
  while(1);
}



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

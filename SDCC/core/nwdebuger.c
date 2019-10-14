#include "nwdebuger.h"

// Настройка выводов SIG и порта uart для отладки
void nwDebugerInit(void)
{

}

__attribute__((weak)) void STACK_FAILURE(char* msg)
{
  LOG(MSG_ON | MSG_ALARM | MSG_ALL, "STACK FAILURE. HALT\n");
  while(1);
}
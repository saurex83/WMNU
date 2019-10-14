#include "NTMR.h"
#include "stdlib.h"

// После вызова EventCallback аппаратное прерывание завершается
// и управление передается в главный цикл main.
// Пользователь сам должен положить систему в сон

// Публичные методы
NT_s* NT_Create(void);
bool NT_Delete(NT_s *nt);

// Методы класса
bool NT_SetTime(uint16_t ticks);
bool NT_SetCapture(uint16_t ticks);
void NT_IRQEnable(bool state);
void NT_SetEventCallback(void (*fn)(uint16_t ticks));
uint16_t NT_GetTime(void);
// Приватные методы

NT_s* NT_Create(void)
{
  // Заполняем структуру указателей
  NT_s* nt = (NT_s*)malloc(NT_S_SIZE);
  nt->NT_SetTime = NT_SetTime;
  nt->NT_SetCapture = NT_SetCapture;
  nt->NT_IRQEnable = NT_IRQEnable;
  nt->NT_SetEventCallback;
  nt->NT_GetTime;
   
  return nt;
}

bool NT_Delete(NT_s *nt)
{
  free(nt);
  return true;
}

bool NT_SetTime(uint16_t ticks)
{
  return false;
}

bool NT_SetCapture(uint16_t ticks)
{
  return false;
}

void NT_IRQEnable(bool state)
{
}

void NT_SetEventCallback(void (*fn)(uint16_t ticks))
{
}

uint16_t NT_GetTime(void)
{
  return 0;
}


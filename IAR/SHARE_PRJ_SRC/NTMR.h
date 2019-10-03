#include "stdbool.h"
#include "stdint.h"

typedef struct
{
  bool (*NT_SetTime)(uint16_t ticks);
  bool (*NT_SetCapture)(uint16_t ticks);
  void (*NT_IRQEnable)(bool state);
  void (*NT_SetEventCallback)(void (*fn)(uint16_t ticks));
  uint16_t (*NT_GetTime)(void);
} NT_s;

NT_s* NT_create(void);
bool NT_delete(NT_s *nt);
#include "stdbool.h"
#include "stdint.h"
#include "NTMR.h"

typedef struct
{
 bool (*TIC_SetTimer)(uint16_t ticks);
 uint16_t (*TIC_GetTimer)(void);
 bool (*TIC_SetTXState)(uint8_t TS, bool state);
 bool (*TIC_SetRXState)(uint8_t TS, bool state);
 bool (*TIC_GetTXState)(uint8_t TS);
 bool (*TIC_GetRXState)(uint8_t TS);
 void (*TIC_SetRXCallback)(void (*fn)(uint8_t TS));
 void (*TIC_SetTXCallback)(void (*fn)(uint8_t TS));
 void (*TIC_SetSECallback)(void (*fn)(uint8_t TS));
 uint32_t (*TIC_getUptime)(void);
 uint32_t (*TIC_getRTC)(void);
 bool (*TIC_setRTC)(uint32_t RTC);
} TIC_s;

TIC_s* TIC_Create(NT_s* nt);
bool TIC_Delete(TIC_s *tic);

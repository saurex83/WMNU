#pragma once
#include "stdbool.h"
#include "stdint.h"
#include "FChain.h"

typedef struct
{
  void (*RI_On)(void);
  void (*RI_Off)(void);
  bool (*RI_SetChannel)(uint8_t CH);
  bool (*RI_Send)(FChain_s *fc);
  FChain_s* (*RI_Receive)(uint8_t timeout);
  uint32_t (*RI_GetCRCError)(void);
  uint32_t (*RI_getCCAReject)(void);
  uint32_t (*RI_getUptime)(void);
} RI_s;

RI_s* RI_create(void);
bool RI_delete(RI_s *ri);
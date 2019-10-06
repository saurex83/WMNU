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
  uint32_t (*RI_GetCCAReject)(void);
  uint32_t (*RI_GetUptime)(void);
  void (*RI_StreamCrypt)(bool state);
} RI_s;

#define RI_S_SIZE sizeof(RI_s)

enum CHANNAL_e
{
  CH11 = 11, CH12 = 12, CH13 = 13, CH14 = 14, CH15 = 15, CH16 = 16,
  CH17 = 17, CH18 = 18, CH19 = 19, CH20 = 21, CH22 = 22, CH23 = 23, 
  CH24 = 24, CH25 = 25, CH26 = 26
};

RI_s* RI_create(void);
bool RI_delete(RI_s *ri);
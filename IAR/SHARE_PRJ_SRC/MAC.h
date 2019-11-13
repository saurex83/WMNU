#pragma once
#include "stdbool.h"
#include "stdint.h"
#include "TIC.h"
#include "RADIO.h"
#include "frame.h"

typedef struct
{
  void (*MAC_OpenRXSlot)(uint8_t TS, uint8_t CH);
  void (*MAC_CloseRXSlot)(uint8_t CH);
  void (*MAC_Send)(frame_s *fr, uint8_t attempts);
  void (*MAC_SetRXCallback)(void (*fn)(frame_s *fr));
  bool (*MAC_GetTXState)(uint8_t TS);
  bool (*MAC_GetRXState)(uint8_t TS);
} MAC_s;

MAC_s* MAC_Create(TIC_s* tic, RI_s* RI);
bool MAC_Delete(MAC_s *mac);
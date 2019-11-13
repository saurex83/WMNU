#pragma once
#include "stdbool.h"
#include "stdint.h"
#include "TIC.h"
#include "RADIO.h"
#include "frame.h"

void MAC_Create(void);
void MAC_OpenRXSlot(uint8_t TS, uint8_t CH);
void MAC_CloseRXSlot(uint8_t CH);
void MAC_Send(frame_s *fr, uint8_t attempts);
void MAC_SetRXCallback(void (*fn)(frame_s *fr));
bool MAC_GetTXState(uint8_t TS);
bool MAC_GetRXState(uint8_t TS);
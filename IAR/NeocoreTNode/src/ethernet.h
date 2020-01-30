#pragma once

#include "frame.h"

void ETH_Init(void);
void ETH_Reset(void);
void ETH_Send(frame_s *fr);
void ETH_SetRXCallback(void (*fn)(frame_s *fr));
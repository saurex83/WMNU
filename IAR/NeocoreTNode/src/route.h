#pragma once
#include "frame.h"

void RP_Init(void);
void RP_Reset(void);
void RP_Send_GW(frame_s *fr);
void RP_Send(frame_s *fr);
void RP_SendRT_GW(frame_s *fr);
void RP_SendRT_RT(frame_s *fr);
void RP_Bind_PID(uint8_t pid, void (*fn)(frame_s *fr));
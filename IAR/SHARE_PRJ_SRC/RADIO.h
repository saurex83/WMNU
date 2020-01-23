#pragma once
#include "stdbool.h"
#include "stdint.h"
#include "frame.h"

enum CHANNAL_e
{
  CH11 = 11, CH12 = 12, CH13 = 13, CH14 = 14, CH15 = 15, CH16 = 16,
  CH17 = 17, CH18 = 18, CH19 = 19, CH20 = 21, CH22 = 22, CH23 = 23, 
  CH24 = 24, CH25 = 25, CH26 = 26
};

void RI_init(void);
bool RI_SetChannel(uint8_t CH);
bool RI_Send(frame_s *fr);
frame_s* RI_Receive(uint16_t timeout);
uint32_t RI_GetCRCError(void);
uint32_t RI_GetCCAReject(void);
float RI_GetUptime(void);
bool RI_Measure_POW(uint8_t fch, uint16_t timeout_ms, int8_t *RSSI_SIG);
#pragma once
#include "frame.h"

void UDP_Init(void);
void UDP_Reset(void);
void UDP_Send(uint8_t port, frame_s *fr);
void UDP_Receive_HNDL(frame_s *fr);
void UDP_Bind(uint8_t port, void (*fn)(frame_s *fr));
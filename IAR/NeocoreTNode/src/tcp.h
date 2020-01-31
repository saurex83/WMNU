#pragma once
#include "frame.h"


void TCP_Init(void);
void TCP_Reset(void);
void TCP_Send(frame_s *fr);
void TCP_Receive_HNDL(frame_s *fr);
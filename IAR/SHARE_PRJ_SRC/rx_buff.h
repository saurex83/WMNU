#include "frame.h"

void RXB_Init(void);
void RXB_Reset(void);
uint8_t RXB_frame_count(void);
frame_s* RXB_get_frame(void);
void RXB_del_frame(frame_s *fr);
#include "MAC.h"
#include "frame.h"

static void MAC_RX_HNDL(uint8_t TS);
static void MAC_TX_HNDL(uint8_t TS);
static bool MAC_Filter(frame_s *fr);
static bool MAC_Decode(frame_s *fr);

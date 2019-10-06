#include "MAC.h"
#include "Frames.h"

static void MAC_RX_HNDL(uint8_t TS);
static void MAC_TX_HNDL(uint8_t TS);
static bool MAC_Filter(FChain_s *fc);
static bool MAC_Decode(FChain_s *fc);

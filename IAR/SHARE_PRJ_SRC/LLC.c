#include "LLC.h"

static void SE_HNDL(uint8_t TS);
static void LLC_Shelduler(uint8_t TS);
static void LLC_RX_HNDL(FChain_s *fc);
static void LLC_RunTimeAlloc(void);
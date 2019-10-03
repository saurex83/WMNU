#include "stdbool.h"
#include "stdint.h"
#include "MAC.h"

typedef struct
{
  void (*LLC_SetMACController)(MAC_s *mac);
  void (*LLC_SetRXCallback)(void (*fn)(FChain_s *fc));
  void (*LLC_TimeAlloc)(void (*fn)(void));
 
} LLC_s;

LLC_s* LLC_create(void);
bool LLC_delete(LLC_s *llc);
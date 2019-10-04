#include "stdbool.h"
#include "stdint.h"
#include "TIC.h"
#include "RADIO.h"
#include "FChain.h"

typedef struct
{
  void (*MAC_OpenRXSlot)(uint8_t TS, uint8_t CH);
  void (*MAC_CloseRXSlot)(uint8_t CH);
  void (*MAC_Send)(FChain_s *fc, uint8_t attempts);
  void (*MAC_SetRXCallback)(FChain_s *fc);
  bool (*MAC_GetTXState)(uint8_t TS);
  bool (*MAC_GetRXState)(uint8_t TS);
} MAC_s;

MAC_s* MAC_Create(TIC_s* tic, RI_s* RI);
bool MAC_Delete(MAC_s *mac);
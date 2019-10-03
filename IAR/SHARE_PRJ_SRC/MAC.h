#include "stdbool.h"
#include "stdint.h"
#include "TIC.h"
#include "FChain.h"

typedef struct
{
  void (*MAC_OpenRXSlot)(uint8_t TS, uint8_t CH);
  void (*MAC_CloseRXSlot)(uint8_t CH);
  void (*MAC_Send)(FChain_s *fc, uint8_t attempts);
  void (*MAC_SetRXCallback)(FChain_s *fc);
  void (*MAC_SetTICController)(TIC_s *tic);
  bool (*MAC_GetTXState)(uint8_t TS);
  bool (*MAC_GetRXState)(uint8_t TS);
  void (*MAC_SetRIController)(void); //TODO
} MAC_s;

MAC_s* MAC_create(void);
bool MAC_delete(MAC_s *mac);
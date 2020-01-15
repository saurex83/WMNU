#include "stdbool.h"

typedef enum MG_state_e
{
  MG_connected, MG_disconnected, MG_disable, MG_TIC_disabled, MG_TIM_disabled,
  MG_AES_disabled, MG_MAC_disabled, MG_LLC_disabled
} MG_state_e;

MG_state_e MG_Net_State();

bool MG_Connect();

void MG_Set_State(MG_state_e MG_state, bool state);
bool MG_Get_State(MG_state_e MG_state);
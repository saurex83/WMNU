#include "stdbool.h"
#include "stdint.h"

void neocore_hw_init();
void network_seed_enable(bool en);
bool get_network_seed_status(void);
bool neocore_stack_reset();
bool network_discovery(uint8_t timeout_ms);

#include "stdbool.h"

int SL_zone_check();
int SL_available();
int SL_busy();
void SL_free(char *buff);
char* SL_alloc(void);
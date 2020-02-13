#include "stdbool.h"


typedef unsigned long ustime_t;
typedef unsigned long stamp_t;

stamp_t UST_now(void);
ustime_t UST_interval(stamp_t beg, stamp_t end);
void UST_delay(ustime_t time);
bool UST_time_over(stamp_t beg, ustime_t wait);
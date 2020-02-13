#pragma once
#include "stdbool.h"

/**
@brief Представление модели модуля
*/
#define AT_MODEL AT
struct AT{
  int a;
};


typedef unsigned int nwtime_t;

void AT_set_time(nwtime_t time);
void AT_set_alarm(nwtime_t time);
nwtime_t AT_time(void);
void AT_wait(nwtime_t time);
void AT_enable(bool state);

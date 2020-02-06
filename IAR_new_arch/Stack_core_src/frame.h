#pragma once
#include "meta.h"

/**
@brief Представление модели модуля
*/
#define FR_MODEL FR
struct FR{
  int a;
};

#define USER_SPACE (space)0
#define IRQ_SPACE (space)1

typedef int frame_id;
typedef char space;

struct frame{
  char slot_flag;
  char len;
  struct meta meta;
  char payload[128];
};

frame_id FR_Create(space sp);


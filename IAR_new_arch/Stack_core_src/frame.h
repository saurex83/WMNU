#pragma once
#include "meta.h"

/**
@brief Представление модели модуля
*/
#define FR_MODEL FR
struct FR{
  int a;
};

#define MAX_PAYLOAD_SIZE 127

struct frame{
  char len;
  struct meta meta;
  char payload[MAX_PAYLOAD_SIZE];
};

struct frame* FR_create();
void FR_delete(struct frame*);
void FR_add_header(struct frame*, void *head, char len);
void FR_del_header(struct frame*, char len);
int FR_busy();
int FR_available();
 


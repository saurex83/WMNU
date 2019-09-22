#include "stdbool.h"
#include "stdint.h"
#include "FItem.h"

typedef struct 
{
	FItem_s* head;
	FItem_s* tail;
	uint8_t  quantity;
	FItem_s* iterator;
} __attribute__((packed)) FChain_s;

bool FC_create(FChain_s* fc);
bool FC_delete(FChain_s* fc);
void FC_iteratorToHead(FChain_s* fc);
void FC_iteratorToTail(FChain_s* fc);
bool FC_iteratorToType(FChain_s* fc, FItem_t type);
bool FC_iteratorToTypeHead(FChain_s* fc, FItem_t type);
void FC_next(FChain_s* fc);
void FC_last(FChain_s* fc);
void FC_insertAfter(FChain_s* fc, FItem_s* fi);
void FC_insertBefore(FChain_s* fc, FItem_s* fi);
uint8_t FC_getQuantity(FChain_s* fc);
bool FC_getIterator(FChain_s* fc, FItem_s* fi);
uint8_t FC_getObjectQuantity();
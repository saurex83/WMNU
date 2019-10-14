#pragma once
#include "stdbool.h"
#include "stdint.h"

typedef enum FItem_t
{
  ETH_H = 1, ETH_DATA = 2, ETH_F = 4, IP_HEADER = 5, IP_DATA_CRYPT = 6, IP_DATA_DECRYPT = 7,
  IP_MIC = 8, SYNC = 9, RAW = 10
} FItem_t;


typedef struct FItem_s
{
	struct FItem_s* next;
	struct FItem_s* last;
	FItem_t 	type;
	uint8_t 	length;
	uint8_t* 	data; 
} __attribute__((packed)) FItem_s;

FItem_s* FI_create(FItem_t type, uint8_t* data, uint8_t length);
bool FI_delete(FItem_s* fi);
bool FI_setNext(FItem_s* fi, FItem_s* fi_next);
bool FI_setLast(FItem_s* fi, FItem_s* fi_last);
FItem_s* FI_getNext(FItem_s* fi);
FItem_s* FI_getLast(FItem_s* fi);
FItem_t FI_getType(FItem_s* fi);
uint8_t FI_getLength(FItem_s* fi);
uint8_t* FI_getData(FItem_s* fi);
uint8_t FI_getObjectQuantity();
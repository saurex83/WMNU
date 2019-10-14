#pragma once
#include "stdbool.h"
#include "stdint.h"
#include "FItem.h"

typedef struct FChain_s FChain_s;
typedef struct Meta_s Meta_s;

struct Meta_s
{
   uint16_t TIMESTAMP;
   int8_t RSSI;
/* link quality. Calculate LQI = (CORR - a)b. Where a and b are found 
   empirically based on PER measurements as a function of the correlation 
   value. PER - packet error rate */
   int8_t LIQ;  
   uint8_t TS;
   uint8_t CH;
   uint8_t PID;
   uint16_t NDST;
   uint16_t NSRC;
   uint8_t ETX;
   uint16_t FDST;
   uint16_t FSRC;
   uint8_t IPP;
   FChain_s* (*SFD_Callback)(void);
};

struct FChain_s
{
	FItem_s* head;
	FItem_s* tail;
	uint8_t  quantity;
	FItem_s* iterator;
        Meta_s   meta;
} __attribute__((packed));

FChain_s* FC_create(void);
bool FC_delete(FChain_s* fc);
void FC_iteratorToHead(FChain_s* fc);
void FC_iteratorToTail(FChain_s* fc);
bool FC_iteratorToType(FChain_s* fc, FItem_t type);
bool FC_iteratorToTypeHead(FChain_s* fc, FItem_t type);
bool FC_next(FChain_s* fc);
bool FC_last(FChain_s* fc);
void FC_insertAfter(FChain_s* fc, FItem_s* fi);
void FC_insertBefore(FChain_s* fc, FItem_s* fi);
bool FC_isHaveType(FChain_s* fc, FItem_t type);
uint8_t FC_getQuantity(FChain_s* fc);
FItem_s* FC_getIterator(FChain_s* fc);
uint8_t FC_getObjectQuantity();
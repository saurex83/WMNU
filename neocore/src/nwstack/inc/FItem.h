#include "stdbool.h"
#include "stdint.h"


typedef enum FItem_t
{
	PPDU_HEADER = 1, PPDU_FOOTER = 2, MPDU_WP = 3, MPDU_MDATA = 4,
	MPDU_MIC = 5, RAW = 6, SYNC = 7, METADATA = 8
} FItem_t;

typedef struct FItem_s
{
	struct FItem_s* 	next;
	struct FItem_s* 	last;
	FItem_t 			type;
	uint8_t 			length;
	uint8_t* 			data; 
} __attribute__((packed)) FItem_s;

bool FI_create(FItem_s* fi, FItem_t type, uint8_t* data, uint8_t length);
bool FI_delete(FItem_s* fi);
bool FI_setNext(FItem_s* fi, FItem_s* fi_next);
bool FI_setLast(FItem_s* fi, FItem_s* fi_last);
bool FI_getNext(FItem_s* fi, FItem_s* fi_next);
bool FI_getLast(FItem_s* fi, FItem_s* fi_last);
FItem_t FI_getType(FItem_s* fi);
uint8_t FI_getLength(FItem_s* fi);
uint8_t* FI_getData(FItem_s* fi);
uint8_t FI_getObjectQuantity();
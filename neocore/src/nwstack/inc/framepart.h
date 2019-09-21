/*
*/

#include "stdbool.h"
#include "stdint.h"

typedef enum framePart_t
{
	PPDU_HEADER = 1, PPDU_FOOTER = 2, MPDU_WP = 3, MPDU_MDATA = 4,
	MPDU_MIC = 5, RAW = 6, SYNC = 7, METADATA = 8
} framePart_t;

typedef struct framePart_s
{
	struct framePart_s* 	next;
	struct framePart_s* 	last;
	framePart_t 			type;
	uint8_t 				part_len;
	uint8_t* 				part_data; 
} framePart_s;

bool FP_create(framePart_s* fp,framePart_t type, 
	uint8_t* part_data, uint8_t part_len);

bool FP_delete(framePart_s* fp);
void FP_getPartLen(framePart_s* fp, uint8_t* part_len);
void FP_getPartData(framePart_s* fp, uint8_t* part_data);
void FP_addNext(framePart_s* fp, framePart_s* next_fp);
void FP_addLast(framePart_s* fp, framePart_s* last_fp);
void FP_deleteChain(framePart_s* fp);
/*
 Модуль работы с кадрами стека
*/

#include "nwdebuger.h"
#include "framepart.h"
#include "stdlib.h"
#include "string.h"

bool FP_create(framePart_s* fp,framePart_t type, 
	uint8_t* part_data, uint8_t part_len);

bool FP_delete(framePart_s* fp);
void FP_getPartLen(framePart_s* fp, uint8_t* part_len);
void FP_getPartData(framePart_s* fp, uint8_t* part_data);
void FP_addNext(framePart_s* fp, framePart_s* next_fp);
void FP_addLast(framePart_s* fp, framePart_s* last_fp);
void FP_deleteChain(framePart_s* fp);


bool FP_create(framePart_s* fp,framePart_t type, 
	uint8_t* part_data, uint8_t part_len)
{
	fp = (framePart_s*)malloc(sizeof(framePart_s));
	fp->type = type;
	fp->part_len = part_len;
	fp->part_data = (uint8_t*)malloc(part_len);
	fp->last=NULL;
	fp->next=NULL;

	ASSERT_HALT(fp != NULL, "Memory allocation fails");
	ASSERT_HALT(fp->part_data != NULL, "Memory allocation fails");

	memcpy(fp->part_data, part_data, part_len);
	return true;
}

bool FP_delete(framePart_s* fp)
{
	ASSERT_HALT(fp != NULL, "Cant free NULL ptr");
	ASSERT_HALT(fp->part_data != NULL, "Cant free NULL ptr");
	
	// Если удаляем элемент из цепочки нужно востановить связи соседей.
	framePart_s* last_neighbor = fp->last;
	framePart_s* next_neighbor = fp->next;
	
	if (last_neighbor != NULL){
		last_neighbor->next = next_neighbor;
	}

	if (next_neighbor != NULL){
		next_neighbor->last = last_neighbor;
	}

	free(fp->part_data);
	free(fp);
	return true;
}

void FP_getPartLen(framePart_s* fp, uint8_t* part_len)
{
	ASSERT_HALT(fp != NULL, "Incorrect FP pointer");
	*part_len = fp->part_len;
}

void FP_getPartData(framePart_s* fp, uint8_t* part_data)
{
	ASSERT_HALT(fp != NULL, "Incorrect FP pointer");
	ASSERT_HALT(fp->part_data != NULL, "Incorrect FP->part_data pointer");
	part_data = fp->part_data;
}

void FP_addNext(framePart_s* fp, framePart_s* next_fp)
{
	ASSERT_HALT(fp != NULL, "Incorrect FP pointer");
	ASSERT_HALT(next_fp != NULL, "Incorrect next FP pointer");
	
	framePart_s* next_neighbor = fp->next;

	// Указатель текущего узла устанавливаем на новый следующий узел
	fp->next = next_fp;
    // Указатель бывшего следующего узла устанавливаем на новый узел
    if (next_neighbor != NULL)
		next_neighbor->last = next_fp;
	else
		next_fp->next = NULL;
}

void FP_addLast(framePart_s* fp, framePart_s* last_fp)
{
	ASSERT_HALT(fp != NULL, "Incorrect FP pointer");
	ASSERT_HALT(last_fp != NULL, "Incorrect last FP pointer");
	
	framePart_s* last_neighbor = fp->last;

	// Указатель текущего узла устанавливаем на новый следующий узел
	fp->last = last_fp;
    // Указатель бывшего следующего узла устанавливаем на новый узел
	if (last_neighbor != NULL)
		last_neighbor->next = last_fp;
	else
		last_fp->last = NULL;
}

static void FP_get_first_in_chain(framePart_s* fp)
{
	while (fp->last != NULL)
		fp = fp->last;
}

void FP_deleteChain(framePart_s* fp)
{
	framePart_s* fp_next;

	FP_get_first_in_chain(fp);

	do
	{
		fp_next = fp->next;
		FP_delete(fp);
		fp = fp_next;
	} while (fp->next != NULL);
}
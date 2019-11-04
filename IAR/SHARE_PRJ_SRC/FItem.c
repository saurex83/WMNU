#include "FItem.h"
#include "nwdebuger.h"
#include "stdlib.h"
#include "string.h"

static uint8_t QUANTITY = 0; // Подсчет обьектов

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

FItem_s* FI_create(FItem_t type, uint8_t* data, uint8_t length)
{
	FItem_s* fi = (FItem_s*)malloc(sizeof(FItem_s));
	
	ASSERT_HALT(fi != NULL, "Memory allocation fails");
	
	if (fi == NULL)
		return NULL;

	fi->next = NULL;
	fi->last = NULL;
	fi->type = type;
	fi->length = length;

	fi->data = (uint8_t*)malloc(length);
	
	ASSERT_HALT(fi->data != NULL, "Memory allocation fails");
	
	if (fi->data == NULL)
	{
		free(fi);
		return NULL;
	}

	memcpy(fi->data, data, length);
	QUANTITY++;
	return fi;
}

bool FI_delete(FItem_s* fi)
{
	ASSERT_HALT(fi != NULL, "Cant free NULL FItem");
	ASSERT_HALT(fi->data != NULL, "Cant free NULL FItem->data");
	free(fi->data);
	free(fi);
    if (QUANTITY > 0)
      QUANTITY--;
	return true;
}

bool FI_setNext(FItem_s* fi, FItem_s* fi_next)
{
	ASSERT_HALT(fi != NULL, "FItem empty");
	if (fi == NULL)
		return false;
	
	ASSERT_HALT(fi_next != NULL, "FItem_next empty");
	if (fi_next == NULL)
		return false;

	fi->next = fi_next;
	fi_next->last = fi;
	return true;
}

bool FI_setLast(FItem_s* fi, FItem_s* fi_last)
{
	ASSERT_HALT(fi != NULL, "FItem empty");
	if (fi == NULL)
		return false;
	
	ASSERT_HALT(fi_last != NULL, "FItem_last empty");
	if (fi_last == NULL)
		return false;

	fi->last = fi_last;
	fi_last->next = fi;
	return true;
}

FItem_s* FI_getNext(FItem_s* fi)
{
	ASSERT_HALT(fi != NULL, "FItem empty");
	if (fi == NULL)
		return NULL;
    
	return fi->next;
}

FItem_s* FI_getLast(FItem_s* fi)
{
	ASSERT_HALT(fi != NULL, "FItem empty");
	if (fi == NULL)
		return NULL;

	return fi->last;
}

FItem_t FI_getType(FItem_s* fi)
{
	ASSERT_HALT(fi != NULL, "FItem empty");
	return fi->type;
}

uint8_t FI_getLength(FItem_s* fi)
{
	ASSERT_HALT(fi != NULL, "FItem empty");
	return fi->length;
}

uint8_t* FI_getData(FItem_s* fi)
{
	ASSERT_HALT(fi != NULL, "FItem empty");
	ASSERT_HALT(fi->data != NULL, "FItem->data empty")
	return fi->data;
}

uint8_t FI_getObjectQuantity()
{
	return QUANTITY;
}



#include "FChain.h"
#include "nwdebuger.h"
#include "stdlib.h"
#include "string.h"

static uint8_t QUANTITY = 0; // Подсчет обьектов

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


bool FC_create(FChain_s* fc)
{
	fc = (FChain_s*)malloc(sizeof(FChain_s));
	
	ASSERT_HALT(fc != NULL, "Memory allocation fails");
	
	if (fc == NULL)
		return false;

	fc->head = NULL;
	fc->tail = NULL;
	fc->quantity = 0;
	fc->iterator = NULL;
	QUANTITY++;
	return true;
}

bool FC_delete(FChain_s* fc)
{
	ASSERT_HALT(fc != NULL, "Cant delete NULL FChain");
	
	if (fc == NULL)
		return false;

	FItem_s* next = NULL;
	FItem_s* item = fc->head;

// Удаляем все элементы цепочки
	while (item != NULL)
	{
		FI_getNext(item, next);
		FI_delete(item);
		item = next;
	}

	free(fc);

	QUANTITY--;
	return false;
}

void FC_iteratorToHead(FChain_s* fc)
{
	fc->iterator = fc->head;
}

void FC_iteratorToTail(FChain_s* fc)
{
	fc->iterator = fc->tail;
}

bool FC_iteratorToType(FChain_s* fc, FItem_t type)
{
	ASSERT_HALT(fc != NULL, "Cant iterate NULL FChain");

	FItem_s* new_iterator = fc->iterator;

	// Итератор может ни на что не указывать
	if (new_iterator == NULL)
		return false;

	while(true)
	{
		if (FI_getType(new_iterator) == type)
		{
			fc->iterator = new_iterator;
			return true;
		}

		FI_getNext(new_iterator, new_iterator);

		// Возможно дошли до конца
		if (new_iterator == NULL)
			return false;
	}
}

bool FC_iteratorToTypeHead(FChain_s* fc, FItem_t type)
{
	ASSERT_HALT(fc != NULL, "Cant iterate NULL FChain");

	// Сохраним предыдущее состояние итератора
	FItem_s* old_iterator = fc->iterator;	
	// Установим итератор на начало цепочки
	fc->iterator = fc->head;
	bool res = FC_iteratorToType(fc, type);

	if (!res)
	{
		fc->iterator = old_iterator;
		return false;
	}

	return true;
}

void FC_next(FChain_s* fc)
{
	ASSERT_HALT(fc != NULL, "Cant get next NULL FChain");
	if (fc->iterator == NULL)
		return;
	FI_getNext(fc->iterator, fc->iterator);
}

void FC_last(FChain_s* fc)
{
	ASSERT_HALT(fc != NULL, "Cant get last NULL FChain");
	if (fc->iterator == NULL)
		return;
	FI_getLast(fc->iterator, fc->iterator);
}

void FC_insertAfter(FChain_s* fc, FItem_s* fi)
{
	ASSERT_HALT(fc != NULL, "Cant insert NULL FChain");
	ASSERT_HALT(fi != NULL, "Cant insert NULL FItem");
	ASSERT_HALT(fc->iterator != NULL, "Cant insert NULL iterator");


	FItem_s* item_next = NULL;
	FItem_s* item_iter = fc->iterator; 
	FI_getNext(item_iter, item_next);

	// К текущем итератору привязываем новый элемент
	FI_setNext(item_iter, fi);
	FI_setLast(fi, item_iter);

	// Если была вставка, то перепривяжим следующий элемент
	if (item_next != NULL)
	{
		FI_setLast(item_next, fi);
		FI_setNext(fi, item_next);
	}
	
	fc->quantity++;
}

void FC_insertBefore(FChain_s* fc, FItem_s* fi)
{
	ASSERT_HALT(fc != NULL, "Cant insert NULL FChain");
	ASSERT_HALT(fi != NULL, "Cant insert NULL FItem");
	ASSERT_HALT(fc->iterator != NULL, "Cant insert NULL iterator");

	FItem_s* item_last = NULL;
	FItem_s* item_iter = fc->iterator; 
	FI_getNext(item_iter, item_last);

	// К текущем итератору привязываем новый элемент
	FI_setLast(item_iter, fi);
	FI_setNext(fi, item_iter);

	// Если была вставка, то перепривяжим следующий элемент
	if (item_last != NULL)
	{
		FI_setNext(item_last, fi);
		FI_setLast(fi, item_last);
	}
	
	fc->quantity++;
}

uint8_t FC_getQuantity(FChain_s* fc)
{
	ASSERT_HALT(fc != NULL, "Cant get quantity NULL FChain");
	return fc->quantity;
}

bool FC_getIterator(FChain_s* fc, FItem_s* fi)
{
	ASSERT_HALT(fc != NULL, "Cant get iterator NULL FChain");
	if (fc->iterator == NULL )
	{
		fi = NULL;
		return false;
	}
	fi = fc->iterator;
	return true;
}

uint8_t FC_getObjectQuantity()
{
	return QUANTITY;
}
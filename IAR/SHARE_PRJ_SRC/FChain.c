#include "FChain.h"
#include "nwdebuger.h"
#include "stdlib.h"
#include "string.h"

static uint8_t QUANTITY = 0; // Подсчет обьектов

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
uint8_t FC_getQuantity(FChain_s* fc);
FItem_s* FC_getIterator(FChain_s* fc);
uint8_t FC_getObjectQuantity();
bool FC_isHaveType(FChain_s* fc, FItem_t type);


FChain_s* FC_create(void)
{
	FChain_s* fc = (FChain_s*)malloc(sizeof(FChain_s));
	
	ASSERT_HALT(fc != NULL, "Memory allocation fails");
	
	if (fc == NULL)
		return NULL;

	fc->head = NULL;
	fc->tail = NULL;
	fc->quantity = 0;
	fc->iterator = NULL;
	QUANTITY++;
	return fc;
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
		next = FI_getNext(item);
		FI_delete(item);
		item = next;
        fc->quantity--;
	}

	free(fc);
    
	return true;
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

		new_iterator = FI_getNext(new_iterator);

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

bool FC_next(FChain_s* fc)
{
	ASSERT_HALT(fc != NULL, "Cant get next NULL FChain");
	if (fc->iterator == NULL)
      return false;

    FItem_s* next = FI_getNext(fc->iterator);
    // Итерируем до последнего элемента
    if ( next == NULL)
        return false;
    
    fc->iterator = next;
    return true;
}

bool FC_last(FChain_s* fc)
{
	ASSERT_HALT(fc != NULL, "Cant get last NULL FChain");
	if (fc->iterator == NULL)
		return false;
    
    FItem_s* last = FI_getLast(fc->iterator);
    
    if ( last == NULL)
        return false;
    
    fc->iterator = last;
    return true;
}

void FC_insertAfter(FChain_s* fc, FItem_s* fi)
{
	ASSERT_HALT(fc != NULL, "Cant insert NULL FChain");
	ASSERT_HALT(fi != NULL, "Cant insert NULL FItem");
    // Если цепочка пустая то итератор указывается на NULL.
    // В этом случаи вставляем первый элемент.
    if (fc->quantity == 0)
    {
      fc->head = fi;
      fc->tail = fi;
      fc->iterator = fc->head;
      fi->last = NULL;
      fi->next = NULL;
      fc->quantity++;
      return;
    }
    
	ASSERT_HALT(fc->iterator != NULL, "Cant insert NULL iterator\n");

	FItem_s* item_iter = fc->iterator; 
	FItem_s* item_next = FI_getNext(item_iter);

	// К текущем итератору привязываем новый элемент
	FI_setNext(item_iter, fi);
	FI_setLast(fi, item_iter);

	// Если была вставка, то перепривяжем следующий элемент
	if (item_next != NULL)
	{
		FI_setLast(item_next, fi);
		FI_setNext(fi, item_next);
	}
    else
    {
      // Если вставили последний элемент, установим хвост.
      fc->tail = fi;
    }
	
	fc->quantity++;
}

void FC_insertBefore(FChain_s* fc, FItem_s* fi)
{
	ASSERT_HALT(fc != NULL, "Cant insert NULL FChain");
	ASSERT_HALT(fi != NULL, "Cant insert NULL FItem");

    // Если цепочка пустая то итератор указывается на NULL.
    // В этом случаи вставляем первый элемент.
    if (fc->quantity == 0)
    {
      fc->head = fi;
      fc->tail = fi;
      fc->iterator = fc->head;
      fi->last = NULL;
      fi->next = NULL;
      fc->quantity++;
      return;
    }

	ASSERT_HALT(fc->iterator != NULL, "Cant insert NULL iterator");

	FItem_s* item_iter = fc->iterator; 
	FItem_s* item_last = FI_getLast(item_iter);

	// К текущем итератору привязываем новый элемент
	FI_setLast(item_iter, fi);
	FI_setNext(fi, item_iter);

	// Если была вставка, то перепривяжим следующий элемент
	if (item_last != NULL)
	{
		FI_setNext(item_last, fi);
		FI_setLast(fi, item_last);
	}
        else
    {
      // Если вставили первый элемент, установим голову.
      fc->head = fi;
    }
	
	fc->quantity++;
}

uint8_t FC_getQuantity(FChain_s* fc)
{
	ASSERT_HALT(fc != NULL, "Cant get quantity NULL FChain");
	return fc->quantity;
}

FItem_s* FC_getIterator(FChain_s* fc)
{
	ASSERT_HALT(fc != NULL, "Cant get iterator NULL FChain");
	return fc->iterator;
}

uint8_t FC_getObjectQuantity()
{
	return QUANTITY;
}

bool FC_isHaveType(FChain_s* fc, FItem_t type)
{
  	ASSERT_HALT(fc != NULL, "Cant iterate NULL FChain");

	// Сохраним предыдущее состояние итератора
	FItem_s* old_iterator = fc->iterator;	
	// Установим итератор на начало цепочки
	fc->iterator = fc->head;
	bool res = FC_iteratorToType(fc, type);
    fc->iterator = old_iterator;
    
	if (!res)
      return false;

	return true;
}
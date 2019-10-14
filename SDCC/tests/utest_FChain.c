#include "utest_suite.h"
#include "FChain.h"
#include "FItem.h"

static void test_create(void)
{
  uint8_t FC_getObjectQuantity();
  umsg("FChain", "FC_getObjectQuantity = 0", FC_getObjectQuantity() == 0);
  
  FChain_s* fc = FC_create();

  umsg("FChain", "FC_getObjectQuantity = 1", FC_getObjectQuantity() == 1);
  
  umsg("FChain", "Chain created", fc != NULL);
  umsg("FChain", "Head NULL", fc->head == NULL);
  umsg("FChain", "Tail NULL", fc->tail == NULL);
  umsg("FChain", "FItem Quantity", fc->quantity == 0);
  umsg("FChain", "Iterator NULL", fc->iterator == NULL);
  umsg("FChain", "FC_getQuantity 0 item", FC_getQuantity(fc) == 0);
  
  FItem_s* item1 = FI_create(ETH_H, "abc", 3);
  FItem_s* item2 = FI_create(RAW, "abc", 3);
  FItem_s* item3 = FI_create(ETH_F, "abc", 3);
 
  // Проверяем вставку первого элемента
  // Итератор указывает на вставленный элемент
  umsg_line("FChain: insert item1");
  FC_insertAfter(fc, item1);
  umsg("FChain", "FC_insertAfter. Insert first item. Head", fc->head == item1);
  umsg("FChain", "FC_insertAfter. Insert first item. Tail", fc->tail == item1);
  umsg("FChain", "FC_insertAfter. Insert first item. Iterator", fc->iterator == item1);
  
  // Вставка второго элемента. Второй будет после первого
  umsg_line("FChain: insert item2 after item1");
  FC_insertAfter(fc, item2);
  umsg("FChain", "FC_insertAfter. First item. Last", item1->last == NULL);
  umsg("FChain", "First item. Next", item1->next == item2);
  umsg("FChain", "Second item. Last", item2->last == item1);
  umsg("FChain", "Second item. Next", item2->next == NULL);
  umsg("FChain", "Chain head", fc->head == item1);
  umsg("FChain", "Chain tail", fc->tail == item2);
  umsg("FChain", "Chain iterator", fc->iterator == item1);
  
  // Вставка третьего элемента. Третий будет перед первым
  umsg_line("FChain: insert item3 before item1");
  FC_insertBefore(fc, item3);
  umsg("FChain", "item3.last", item3->last == NULL);
  umsg("FChain", "item3.next", item3->next == item1);
  umsg("FChain", "item1.last", item1->last == item3);
  umsg("FChain", "Chain head", fc->head == item3);
  umsg("FChain", "Chain iterator", fc->iterator == item1);
  
  umsg("FChain", "FC_getQuantity 3 item", FC_getQuantity(fc) == 3);
  
  fc->iterator = NULL;
  
  FC_iteratorToHead(fc);
  umsg("FChain", "FC_iteratorToHead", fc->iterator == item3);
  
  FC_iteratorToTail(fc);
  umsg("FChain", "FC_iteratorToTail", fc->iterator == item2);
  
  umsg("FChain", "FC_getIterator", FC_getIterator(fc) == item2);
  
  // Итерируем с головы к хвосту
  umsg_line("FChain: Iterate form head to tail");
  bool iter_res;
  FC_iteratorToHead(fc);
  umsg("FChain", "FC_getIterator item3", FC_getIterator(fc) == item3);
  
  iter_res = FC_next(fc);
  umsg("FChain", "FC_getIterator item1", FC_getIterator(fc) == item1);
  umsg("FChain", "FC_next return true", iter_res == true);
  
  iter_res = FC_next(fc);
  umsg("FChain", "FC_getIterator item2", FC_getIterator(fc) == item2);
  umsg("FChain", "FC_next return true", iter_res == true);
  
  iter_res = FC_next(fc);
  umsg("FChain", "FC_getIterator item2", FC_getIterator(fc) == item2);
  umsg("FChain", "FC_next return false", iter_res == false);
  
  iter_res = FC_next(fc);
  umsg("FChain", "FC_next return false", iter_res == false);
  
  // Итерируем с хвоста к голове
  umsg_line("FChain: Iterate form tail to head");
  FC_iteratorToTail(fc);
  umsg("FChain", "FC_getIterator item2", FC_getIterator(fc) == item2);
  
  iter_res = FC_last(fc);
  umsg("FChain", "FC_getIterator item1", FC_getIterator(fc) == item1);
  umsg("FChain", "FC_last return true", iter_res == true);
  
  iter_res = FC_last(fc);
  umsg("FChain", "FC_getIterator item3", FC_getIterator(fc) == item3);
  umsg("FChain", "FC_last return true", iter_res == true);
  
  iter_res = FC_last(fc);
  umsg("FChain", "FC_getIterator item3", FC_getIterator(fc) == item3);
  umsg("FChain", "FC_last return false", iter_res == false);
  
  iter_res = FC_last(fc);
  umsg("FChain", "FC_last return false", iter_res == false);
  
  //Поиск элементов ETH_H, RAW, ETH_F
  umsg_line("FChain: FC_iteratorToTypeHead");
  bool search_res;
 
  search_res = FC_iteratorToTypeHead(fc, ETH_H);  
  umsg("FChain", "Search result ETH_H", search_res == true);
  umsg("FChain", "Search result ETH_H", FC_getIterator(fc) == item1);
  
  search_res = FC_iteratorToTypeHead(fc, RAW);  
  umsg("FChain", "Search result RAW", search_res == true);
  umsg("FChain", "Search result RAW", FC_getIterator(fc) == item2);

  search_res = FC_iteratorToTypeHead(fc, ETH_F);  
  umsg("FChain", "Search result ETH_F", search_res == true);
  umsg("FChain", "Search result ETH_F", FC_getIterator(fc) == item3);
  
  //Поиск элементов PPDU_HEADER, RAW, ETH_F
  // Ищем по порядку. Порядок в цеочке item3->item1->item2
  umsg_line("FChain: FC_iteratorToType");
  FC_iteratorToHead(fc);
  
  search_res = FC_iteratorToType(fc, ETH_H);
  umsg("FChain", "Search result ETH_H", search_res == true);
  umsg("FChain", "Search result ETH_H", FC_getIterator(fc) == item1);

  search_res = FC_iteratorToType(fc, RAW);
  umsg("FChain", "Search result RAW", search_res == true);
  umsg("FChain", "Search result RAW", FC_getIterator(fc) == item2);  
  
  // Этот элемент не найдем. так как он находится раньше item2
  search_res = FC_iteratorToType(fc, ETH_F);
  umsg("FChain", "Search result ETH_F. Missed.", search_res == false);
  umsg("FChain", "Search result ETH_F. Missed.", FC_getIterator(fc) != item3); 
 
  // Сбросим указатель на начало, теперь должны найти item3
  FC_iteratorToHead(fc);
  search_res = FC_iteratorToType(fc, ETH_F);
  umsg("FChain", "Search result ETH_F.", search_res == true);
  umsg("FChain", "Search result ETH_F.", FC_getIterator(fc) == item3);
  
  //Удаляем всю цепочку
  umsg_line("FChain: FC_isHaveType");
  umsg("FChain", "FC_isHaveType RAW", FC_isHaveType(fc, RAW));
  umsg("FChain", "FC_isHaveType ETH_F", FC_isHaveType(fc, ETH_F));
  umsg("FChain", "FC_isHaveType ETH_H", FC_isHaveType(fc, ETH_H));
  umsg("FChain", "FC_isHaveType SYNC none", FC_isHaveType(fc, SYNC) == false);  
  
  //Удаляем всю цепочку
  umsg_line("FChain: Delete chain");
  bool del_result = FC_delete(fc);
  umsg("FChain", "Delete FChain.", del_result == true);
  umsg("FChain", "FC_getQuantity 0 item", FC_getQuantity(fc) == 0);
  
  // Добавить в будущем
  umsg("FChain", "TODO memory malloc and free", false);
}


void suite_FChain(void)
{
  umsg_line("FChain module");
  test_create();
}
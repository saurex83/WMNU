#include "utest_suite.h"
#include "FItem.h"


static void test_create(void)
{
  uint8_t data[] = {77,1,2,3,4,5,6,7,8,9};
  uint8_t data_size = sizeof(data);
  FItem_t type = PPDU_HEADER;
  
  FItem_s* fi = FI_create(type, data, data_size);
  umsg("FItem", "Item created", fi != NULL);
  umsg("FItem", "Field next NULL", fi->next == NULL);
  umsg("FItem", "Field last NULL", fi->last == NULL);
  umsg("FItem", "Field type", fi->type == type);
  umsg("FItem", "Field length", fi->length == data_size);
  
  bool mem_compare_res =  memory_compare(
    (char*)fi->data, (char*)data, data_size);
  umsg("FItem", "Frame data check",mem_compare_res == true);
  
  bool fitem_delete = FI_delete(fi);
  umsg("FItem", "Frame delete" ,fitem_delete == true);
}

static void test_frame_manipulation(void)
{
  uint8_t data[] = {1,2,3,4,5};
  uint8_t data_size = sizeof(data);
  
  FItem_s* fi = FI_create(PPDU_HEADER, data, data_size);
  FItem_s* fi_last = FI_create(PPDU_FOOTER, data, data_size);
  FItem_s* fi_next = FI_create(RAW , data, data_size);
  
  FI_setNext(fi, fi_next);
  FI_setLast(fi, fi_last);
  
  FItem_s* geted_fi_next = FI_getNext(fi);
  FItem_s* geted_fi_last = FI_getLast(fi);
  
  umsg("FItem", "Set next frame" ,geted_fi_next == fi_next);
  umsg("FItem", "Set last frame" ,geted_fi_last == fi_last);
  umsg("FItem", "FI_getType next frame" , FI_getType(geted_fi_next) == RAW);
  umsg("FItem", "FI_getType last frame" , FI_getType(geted_fi_last) == PPDU_FOOTER);
  umsg("FItem", "FI_getData" , FI_getData(fi) == fi->data);
  umsg("FItem", "FI_getObjectQuantity create" , FI_getObjectQuantity() == 3);
  
  FI_delete(fi);
  FI_delete(fi_next);
  FI_delete(fi_last);
  
  umsg("FItem", "FI_getObjectQuantity deleted" , FI_getObjectQuantity() == 0);
}

static void test_mem_malloc(void)
{
  umsg("FItem", "TODO memory malloc and free" , false);
}

void suite_FItem(void)
{
  umsg_line("FItem module");
  test_create();
  test_frame_manipulation();
  test_mem_malloc();
}
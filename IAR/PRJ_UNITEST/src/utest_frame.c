#include "utest_suite.h"
#include "frame.h"
#include "stdbool.h"
#include "mem.h"

static void test_1(void)
{
  frame_s *fr = frame_create();
  umsg("frame", "Create", frame_getCount() == 1);
    
  frame_delete(fr);
  umsg("frame", "Delete", frame_getCount() == 0);
}

static void test_2(void)
{
  uint8_t data[5];
  uint16_t heap_ptr_before = heap_ptr(1);
  
  frame_s *fr = frame_create();
  frame_addHeader(fr, data, sizeof(data));
  frame_delete(fr);
  
  umsg("frame", "Memmory free test 1", heap_ptr_before == heap_ptr(1));
}

static void test_3(void)
{
  uint8_t data1[5] = {0,1,2,3,4};
  uint8_t data2[5] = {5,6,7,8,9};
  
  frame_s *fr = frame_create();
  frame_addHeader(fr, data2, sizeof(data2));
  frame_addHeader(fr, data1, sizeof(data1));
  
  bool ok = true;
  uint8_t *beg = (uint8_t*)fr->payload;
  for (int i = 0 ; i < 10; i++)
    if (beg[i] != i)
      ok = false;
  
  umsg("frame", "Header add", ok == true);
  
  frame_delete(fr); 
}

static void test_4(void)
{
  uint8_t data1[] = {0,1,2,3,4,3,87,21,35,73};
  uint8_t data2[] = {5,6,7,8,9,36,85,98,23,98,42,12};
  uint8_t data3[] = {10,11,12,13,14,15,17,20};
  uint8_t data4[] = {5,6,7,8,9,23,56,23,6};
  
  uint16_t heap_ptr_before = heap_ptr(1);
  
  frame_s *fr = frame_create();
  frame_addHeader(fr, data1, sizeof(data1));
  frame_addHeader(fr, data2, sizeof(data2));
  frame_addHeader(fr, data3, sizeof(data3));
  frame_addHeader(fr, data4, sizeof(data4));
 
  frame_delete(fr);
  
  umsg("frame", "Memmory free test 2", heap_ptr_before == heap_ptr(1));
  
   
}

void suite_frame(void)
{
  umsg_line("frame module");
  test_1();
  test_2();
  test_3();
  test_4();
}
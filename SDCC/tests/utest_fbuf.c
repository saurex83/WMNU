#include "utest_suite.h"
#include "fbuf.h"
#include "Net_frames.h"

static void test_create(void)
{ 
  fbuf_s *fb = fbuf_create(FB_ETH_LAY, NULL, NULL);
  umsg("fbuf", "fbuf FB_ETH_LAY created", fb != NULL);
  umsg("fbuf", "fbuf FB_ETH_LAY lenght", fb->len == ETH_LAY_SIZE);
  fbuf_delete(fb);
 
  fb = fbuf_create(FB_IP_LAY, NULL, NULL);
  umsg("fbuf", "fbuf FB_IP_LAY created", fb != NULL);
  umsg("fbuf", "fbuf FB_IP_LAY lenght", fb->len == IP_LAY_SIZE);
  fbuf_delete(fb);
  
  fb = fbuf_create(FB_SYNC_LAY, NULL, NULL);
  umsg("fbuf", "fbuf FB_SYNC_LAY created", fb != NULL);
  umsg("fbuf", "fbuf FB_SYNC_LAY lenght", fb->len == SYNC_LAY_SIZE);
  fbuf_delete(fb);
}

static void test_logic(void)
{
  fbuf_s *fb1 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb2 = fbuf_create(FB_IP_LAY, NULL, NULL);
  fbuf_s *fb3 = fbuf_create(FB_SYNC_LAY, NULL, NULL);
  
  fbuf_chain(fb1, fb2);
  fbuf_chain(fb2, fb3);
  
  umsg("fbuf", "chain fb1 -> fb2", fb1->next == fb2);
  umsg("fbuf", "chain fb2 -> fb3", fb2->next == fb3);
  umsg("fbuf", "chain fb3 next NULL", fb3->next == NULL);
  
  fbuf_s *iterator = fb1;
  umsg("fbuf", "iterator = fb1", iterator == fb1);
  
  iterator = fbuf_next(iterator);
  umsg("fbuf", "iterator = fb2", iterator == fb2);
  
  iterator = fbuf_next(iterator);
  umsg("fbuf", "iterator = fb3", iterator == fb3);
  
  iterator = fbuf_next(iterator);
  umsg("fbuf", "iterator = NULL", iterator == NULL);
  
  fbuf_delete(fb1);
  fbuf_delete(fb2);
  fbuf_delete(fb3);
}

static void raw_data_test(void)
{
  #define RAW_LEN 100 
  uint8_t data[RAW_LEN];
  
  for (uint8_t i = 0; i < RAW_LEN; i++)
    data[i] = i;
  
  fbuf_s *fb = fbuf_create(FB_RAW_LAY, data, RAW_LEN);
  bool flag = memory_compare((char*)data, fb->payload, RAW_LEN);
  umsg("fbuf", "FB_RAW_LAY mem is equal", flag == true);
  
  ((uint8_t*)fb->payload)[6] = 10;
  flag = memory_compare((char*)data, fb->payload, RAW_LEN);
  umsg("fbuf", "FB_RAW_LAY mem is not qual", flag == false);
  
  umsg("fbuf", "FB_RAW_LAY len is equal", fb->len == RAW_LEN);
  fbuf_delete(fb);
}

static void delete_test(void)
{
  fbuf_s *fb = fbuf_create(FB_SYNC_LAY, NULL, NULL);
  fbuf_delete(fb);
  fbuf_s *fb1 = fbuf_create(FB_SYNC_LAY, NULL, NULL);
  umsg("fbuf", "fbuf_delete", fb == fb1);
}

void suite_fbuf(void)
{
  umsg_line("fbuf module");
  test_create();
  delete_test();
  raw_data_test();
  test_logic();
}
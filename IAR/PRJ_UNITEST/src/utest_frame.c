#include "utest_suite.h"
#include "frame.h"
#include "Net_frames.h"

static void insert1_test(void)
{
  fbuf_s *fb1 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  
  frame_s* fr = frame_create();
  frame_insert_head(fr, fb1);
  umsg("frame", "Insert head. head = fb1", fr->head == fb1);
  umsg("frame", "Insert head. tail = fb1", fr->tail == fb1);
  frame_delete(fr);
}

static void insert2_test(void)
{
  fbuf_s *fb1 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  
  frame_s *fr = frame_create();
  frame_insert_tail(fr, fb1);
  umsg("frame", "Insert tail. head = fb1", fr->head == fb1);
  umsg("frame", "Insert tail. tail = fb1", fr->tail == fb1);
  frame_delete(fr);
}

static void insert3_test(void)
{
  fbuf_s *fb1 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb2 = fbuf_create(FB_IP_LAY, NULL, NULL);
  fbuf_s *fb3 = fbuf_create(FB_SYNC_LAY, NULL, NULL);

  frame_s *fr = frame_create();
  frame_insert_head(fr, fb1);
  frame_insert_head(fr, fb2);
  frame_insert_head(fr, fb3);
  
  fbuf_s *iterator = frame_get_fbuf_head(fr);
  umsg("frame", "iterator = fb3", iterator == fb3);
  
  iterator = fbuf_next(iterator);
  umsg("frame", "iterator = fb2", iterator == fb2);
  
  iterator = fbuf_next(iterator);
  umsg("frame", "iterator = fb1", iterator == fb1); 
  
  iterator = fbuf_next(iterator);
  umsg("frame", "iterator = NULL", iterator == NULL);
  
  fbuf_s *tail = frame_get_fbuf_tail(fr);
  umsg("frame", "iterator = fb1", tail == fb1);
  
  frame_delete(fr);

}

static void delete_test(void)
{
  // Тест основан на том что malloc выделит ранее освобожденные
  // участки памяти после освобождения памяти. 
  fbuf_s *fb01 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb02 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb03 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb04 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  
  frame_s *fr = frame_create();
  frame_insert_head(fr, fb01);
  frame_insert_head(fr, fb02);
  frame_insert_head(fr, fb03);
  frame_insert_head(fr, fb04);
  frame_delete(fr);
  
  fbuf_s *fb11 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb12 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb13 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb14 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  
  umsg("frame", "delete test. fb1", fb01 == fb11);
  umsg("frame", "delete test. fb2", fb02 == fb12);
  umsg("frame", "delete test. fb3", fb03 == fb13);
  umsg("frame", "delete test. fb4", fb04 == fb14);
  
  fbuf_delete(fb11);
  fbuf_delete(fb12);
  fbuf_delete(fb13);
  fbuf_delete(fb14);
  fbuf_delete(fb01);
  fbuf_delete(fb02);
  fbuf_delete(fb03);
  fbuf_delete(fb04);
}

static void insert4_test(void)
{
  fbuf_s *fb1 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb2 = fbuf_create(FB_IP_LAY, NULL, NULL);
  fbuf_s *fb3 = fbuf_create(FB_SYNC_LAY, NULL, NULL);

  frame_s *fr = frame_create();
  frame_insert_tail(fr, fb1);
  frame_insert_tail(fr, fb2);
  frame_insert_tail(fr, fb3);
  
  fbuf_s *iterator = frame_get_fbuf_head(fr);
  umsg("frame", "iterator = fb1", iterator == fb1);
  
  iterator = fbuf_next(iterator);
  umsg("frame", "iterator = fb2", iterator == fb2);
  
  iterator = fbuf_next(iterator);
  umsg("frame", "iterator = fb3", iterator == fb3); 
  
  iterator = fbuf_next(iterator);
  umsg("frame", "iterator = NULL", iterator == NULL);
  
  fbuf_s *tail = frame_get_fbuf_head(fr);
  umsg("frame", "iterator = fb1", tail == fb1);
  
  frame_delete(fr);
}

static void tot_len_test(void)
{
  uint8_t tmp[10];
  fbuf_s *fb1 = fbuf_create(FB_ETH_LAY, NULL, NULL);
  fbuf_s *fb2 = fbuf_create(FB_IP_LAY, NULL, NULL);
  fbuf_s *fb3 = fbuf_create(FB_SYNC_LAY, NULL, NULL);
  fbuf_s *fb4 = fbuf_create(FB_RAW_LAY, tmp, sizeof(tmp));
  
  frame_s *fr = frame_create();
  frame_insert_tail(fr, fb1);
  frame_insert_tail(fr, fb2);
  frame_insert_tail(fr, fb3);
  frame_insert_tail(fr, fb4);
  
  uint8_t data_len = ETH_LAY_SIZE + IP_LAY_SIZE + SYNC_LAY_SIZE + sizeof(tmp);
  uint8_t tot_len = frame_len(fr);
  umsg("frame", "frame_len calc", tot_len == data_len);
  frame_delete(fr);
}

static void create_test(void)
{
}

void suite_frame(void)
{
  umsg_line("fbuf module");
  tot_len_test();
  delete_test();
  insert1_test();
  insert2_test();
  insert3_test();
  insert4_test();  
}
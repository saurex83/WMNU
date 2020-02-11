#include "utest.h"
#include "stddef.h"

static int test1(void);
struct test_unit UT_BF_1 = {.name = "BF:push", .fun = test1};

#include "buffer.h"
#include "frame.h"

//bool BF_next_tx(struct frame* frame);
//bool BF_remove_tx(struct frame *frame);
//bool BF_remove_rx(struct frame *frame);
//bool BF_push_rx(struct frame *frame);
//bool BF_push_tx(struct frame *frame);

static int test1(void){
  int test_res = 0;
  struct frame *fr = FR_create();
  
  if (!BF_push_tx(fr))
    test_res = 1;

  if (!BF_push_rx(fr))
    test_res = 2;
  
  // Должен извлечся тот же самый фрейм
  struct frame *pop_fr = NULL;
  pop_fr = BF_next_tx(pop_fr);
  if (!pop_fr)
    test_res = 3;
    
  if (pop_fr != fr)
    test_res = 4;
  
  if (!BF_remove_tx(fr))
    test_res = 5;
  
  // ничего не должно извлечся
  pop_fr = NULL;
  pop_fr = BF_next_tx(pop_fr);
  if (pop_fr)
    test_res = 6;
  
//  // Должен извлечся тот же самый фрейм
//  pop_fr = NULL;
//  if (!BF_next_rx(pop_fr))
//    test_res = 6;
//  
//  if (pop_fr != fr)
//    test_res = 7;
//  
//  // ничего не должно извлечся
//  pop_fr = NULL;
//  if (BF_next_rx(pop_fr))
//    test_res = 8;
  
  return test_res;
}
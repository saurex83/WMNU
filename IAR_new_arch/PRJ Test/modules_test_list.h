#pragma once
#include "utest.h"

#define TEST_FUN_DEF {                          \
  &UT_MS_1, &UT_MS_2, &UT_MS_3,                 \
  &UT_FR_1, &UT_FR_2, &UT_FR_3,                 \
  &UT_BF_1                                      \
}


extern struct test_unit 
  UT_MS_1, UT_MS_2, UT_MS_3,
  UT_FR_1, UT_FR_2, UT_FR_3,
  UT_BF_1;

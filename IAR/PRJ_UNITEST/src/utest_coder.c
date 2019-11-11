#include "utest_suite.h"
#include "coder.h"
#include "delays.h"
#include "string.h"
#include "stdio.h"

static void aes_test(void)
{
  AES_init();
  
  TimeStamp_s start,stop;
  uint32_t passed;
  #define max_len 10 
  uint8_t data[max_len] = "0123456789";
  uint8_t chiper[max_len];
  
  uint8_t key[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  uint8_t nonce[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  TIM_init();
  
  memset(chiper, 0 , max_len);
  
  TIM_TimeStamp(&start);
  AES_StreamCoder(true, data, chiper, key, nonce, max_len);
  TIM_TimeStamp(&stop);
  passed = TIM_passedTime(&start, &stop);
  printf("passed %lu \r\n", passed);
  
  memset(data, 0 , max_len);
  TIM_TimeStamp(&start);
  AES_StreamCoder(false, chiper, data, key, nonce, max_len);
  TIM_TimeStamp(&stop);
  passed = TIM_passedTime(&start, &stop);

  return;
}


//  umsg("FChain", "TODO memory malloc and free", false);
void suite_coder(void)
{
  umsg_line("FChain module");
  aes_test();
}
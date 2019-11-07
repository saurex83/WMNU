#include "utest_suite.h"
#include "coder.h"


static void aes_test(void)
{
  uint8_t data[] = {1,2,3,4,5,6,7,8};
  uint8_t key[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  
  AES_ECB_Encrypt(data, key, sizeof(data));
  return;
}


//  umsg("FChain", "TODO memory malloc and free", false);
void suite_coder(void)
{
  umsg_line("FChain module");
  aes_test();
}
#include "utest_suite.h"
#include "coder.h"
#include "delays.h"
#include "string.h"
#include "stdio.h"
#include "nwdebuger.h"

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

static void CCM_test(void)
{
  AES_init();
  TIM_init();
  
  #define DATA_LEN 10
  uint8_t data[DATA_LEN];
  uint8_t key[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  uint8_t nonce[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  uint8_t MIC[4];
  
  memset(MIC, 0, 4);
  for (uint8_t i = 0; i < DATA_LEN; i++)
    data[i] = i ;
  
  AES_CCMEncrypt( data, 7, 3, 1, key, nonce, MIC);
  
  bool res = AES_CCMDecrypt( data, 7, 3, 1, key, nonce, MIC);
}

static void single_speed_test(uint8_t *data, uint8_t f, uint8_t c, uint8_t m,
                              uint8_t *key, uint8_t *nonce, uint8_t *MIC)
{
  TimeStamp_s start,stop;
  TIM_TimeStamp(&start);
  AES_CCMEncrypt( data, f, c , m, key, nonce, MIC);
  TIM_TimeStamp(&stop);
  uint32_t time = TIM_passedTime(&start, &stop);
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "m=%d f=%d c=%d. Time = %lu\n",
      m, f, c, time);
}

static void CCM_speed_test(void)
{
  #define SPEED_DATA_LEN 100
  uint8_t data[SPEED_DATA_LEN];

  uint8_t key[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  uint8_t nonce[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  uint8_t MIC[16];

  AES_init();
  TIM_init();
  
  for (uint8_t i = 10 ; i <= 50 ; i +=10)
    single_speed_test(data, i, 10, 1, key, nonce, MIC);
  
  for (uint8_t i = 10 ; i <= 50 ; i +=10)
    single_speed_test(data, 10, i, 1, key, nonce, MIC);
  
  for (uint8_t i = 10 ; i <= 50 ; i +=10)
    single_speed_test(data, i, 10, 2, key, nonce, MIC);
  
  for (uint8_t i = 10 ; i <= 50 ; i +=10)
    single_speed_test(data, 10, i, 2, key, nonce, MIC);
}

static bool single_enc_dec_test(uint8_t *data, uint8_t f, uint8_t c, uint8_t m,
                              uint8_t *key, uint8_t *nonce, uint8_t *MIC)
{
  AES_CCMEncrypt( data, f, c, m, key, nonce, MIC);  
  bool res = AES_CCMDecrypt( data, f, c, m, key, nonce, MIC); 
  return res;
}

static void CCM_code_decode_test(void)
{
  #define CODE_DATA_LEN 128
  uint8_t data[CODE_DATA_LEN];

  uint8_t key[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  uint8_t nonce[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  uint8_t MIC[16];

  AES_init();
  TIM_init();  
  
  bool res;
 
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "CCM full size test 60 sec\n");
  
  res = true;
  for (uint8_t m = 0; m < 3; m++)
    for (uint8_t f = 0; f < 128 ; f++)
      for (uint8_t c = 0; c < (128 - f) ; c++) 
      {
        res = res && single_enc_dec_test(data, f, c, m, key, nonce, MIC);
        if (res)
          continue;
        
        LOG(MSG_ON | MSG_INFO | MSG_TRACE, 
            "Coder test stub on m=%d f=%d c=%d \n", m, f, c);
        break;
      }
  umsg("coder", "CCM full coder test", res == true);
  
  res = single_enc_dec_test(data, 10, 10, 1, key, nonce, MIC);
}

static void mem_control(void)
{
  #define MEM_CNTR_SIZE 10
  #define MAGIC_NUM 0x73
  
  uint8_t key[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  uint8_t nonce[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
  
  struct 
  {
    uint8_t cnt_byte1;
    uint8_t data[MEM_CNTR_SIZE];
    uint8_t cnt_byte2;
  } __attribute__((packed)) data_struct;
 
    struct 
  {
    uint8_t cnt_byte1;
    uint8_t key[16];
    uint8_t cnt_byte2;
  } __attribute__((packed)) key_struct;
  
  struct 
  {
    uint8_t cnt_byte1;
    uint8_t nonce[16];
    uint8_t cnt_byte2;
  } __attribute__((packed)) nonce_struct;
  
  struct 
  {
    uint8_t cnt_byte1;
    uint8_t MIC[16];
    uint8_t cnt_byte2;
  } __attribute__((packed)) mic_struct;
  
  AES_init();
  TIM_init();
  
  data_struct.cnt_byte1 = MAGIC_NUM;
  data_struct.cnt_byte2 = MAGIC_NUM;
  key_struct.cnt_byte1 = MAGIC_NUM;  
  key_struct.cnt_byte2 = MAGIC_NUM;
  nonce_struct.cnt_byte1 = MAGIC_NUM;  
  nonce_struct.cnt_byte2 = MAGIC_NUM;
  mic_struct.cnt_byte1 = MAGIC_NUM;  
  mic_struct.cnt_byte2 = MAGIC_NUM;
  
  memcpy(key_struct.key , key, 16);
  memcpy(nonce_struct.nonce , nonce, 16);
  for (uint8_t i = 0; i < MEM_CNTR_SIZE; i++)
    data_struct.data[i] = i;

  for (uint8_t i = 0; i < 16; i++)
  {
    key_struct.key[i] = i + 100;
    nonce_struct.nonce[i] = i + 200;
  }
  
  AES_CCMEncrypt( data_struct.data, 5, MEM_CNTR_SIZE - 5, 1, key_struct.key,
                 nonce_struct.nonce, mic_struct.MIC);  
  
  bool data_mem_corrupt = false;
  bool key_mem_corrupt = false;
  bool nonce_mem_corrupt = false;
  bool mic_mem_corrupt = false;
  if ((data_struct.cnt_byte1 != MAGIC_NUM) || 
      (data_struct.cnt_byte2 != MAGIC_NUM))
  data_mem_corrupt = true;
  if ((key_struct.cnt_byte1 != MAGIC_NUM) || 
      (key_struct.cnt_byte2 != MAGIC_NUM))
  key_mem_corrupt = true;
  if ((nonce_struct.cnt_byte1 != MAGIC_NUM) || 
      (nonce_struct.cnt_byte2 != MAGIC_NUM))
  nonce_mem_corrupt = true;  
  if ((mic_struct.cnt_byte1 != MAGIC_NUM) || 
      (mic_struct.cnt_byte2 != MAGIC_NUM))
  mic_mem_corrupt = true;  
  
  umsg("coder", "CCM coder corrupt data memory", data_mem_corrupt == false);
  umsg("coder", "CCM coder corrupt key memory", key_mem_corrupt == false);
  umsg("coder", "CCM coder corrupt nonce memory", nonce_mem_corrupt == false);
  umsg("coder", "CCM coder corrupt mic memory", mic_mem_corrupt == false);
  
  
  data_struct.cnt_byte1 = MAGIC_NUM;
  data_struct.cnt_byte2 = MAGIC_NUM;
  key_struct.cnt_byte1 = MAGIC_NUM;  
  key_struct.cnt_byte2 = MAGIC_NUM;
  nonce_struct.cnt_byte1 = MAGIC_NUM;  
  nonce_struct.cnt_byte2 = MAGIC_NUM;
  mic_struct.cnt_byte1 = MAGIC_NUM;  
  mic_struct.cnt_byte2 = MAGIC_NUM;
  
  bool res = AES_CCMDecrypt( data_struct.data, 5, MEM_CNTR_SIZE - 5, 1, key_struct.key,
                 nonce_struct.nonce, mic_struct.MIC); 
  
  if ((data_struct.cnt_byte1 != MAGIC_NUM) || 
      (data_struct.cnt_byte2 != MAGIC_NUM))
  data_mem_corrupt = true;
  if ((key_struct.cnt_byte1 != MAGIC_NUM) || 
      (key_struct.cnt_byte2 != MAGIC_NUM))
  key_mem_corrupt = true;
  if ((nonce_struct.cnt_byte1 != MAGIC_NUM) || 
      (nonce_struct.cnt_byte2 != MAGIC_NUM))
  nonce_mem_corrupt = true;  
  if ((mic_struct.cnt_byte1 != MAGIC_NUM) || 
      (mic_struct.cnt_byte2 != MAGIC_NUM))
  mic_mem_corrupt = true;

  umsg("coder", "CCM decoder corrupt data memory", data_mem_corrupt == false);
  umsg("coder", "CCM decoder corrupt key memory", key_mem_corrupt == false);
  umsg("coder", "CCM decoder corrupt nonce memory", nonce_mem_corrupt == false);
  umsg("coder", "CCM decoder corrupt mic memory", mic_mem_corrupt == false);
  
  bool not_changed = true;
  for (uint8_t i = 0; i < MEM_CNTR_SIZE; i++)
    if (data_struct.data[i] != i)
      not_changed = true;
  umsg("coder", "CCM decoded data correct", not_changed == true);
    
  for (uint8_t i = 0; i < 16; i++)
    if (key_struct.key[i] != i + 100)
      not_changed = true;
  umsg("coder", "CCM key data correct", not_changed == true);  
 
  for (uint8_t i = 0; i < 16; i++)
    if (nonce_struct.nonce[i] != i + 200)
      not_changed = true;
  umsg("coder", "CCM nonce data correct", not_changed == true); 
}

//  umsg("FChain", "TODO memory malloc and free", false);
void suite_coder(void)
{
  umsg_line("coder module");
//  CCM_speed_test();
  mem_control();
  CCM_code_decode_test();
  CCM_test();
  aes_test();
}
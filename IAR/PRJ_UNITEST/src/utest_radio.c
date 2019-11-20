#include "utest_suite.h"
#include "RADIO.h"
#include "ioCC2530.h"
#include "delays.h"
#include "frame.h"
#include "coder.h"
#include "nwdebuger.h"
#include "TIC.h"
#include "NTMR.h"

static void RadioSendTest1(void)
{
  uint8_t DATA_SEND[10] = {0,1,2,3,4,5,6,7,8,9};
  uint8_t key[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  uint8_t nonce[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  
  TIM_init();
  RI_init();
  AES_init();
  
  RI_StreamCrypt(true);
  RI_SetChannel(CH11);
  RI_setKEY(key);
  RI_setIV(nonce);
  
  // Подготовим данные к отправке
  frame_s *fr = frame_create();
  fbuf_s *fb = fbuf_create(FB_RAW_LAY, DATA_SEND, sizeof(DATA_SEND));
  fr->meta.SEND_TIME = 3000;
  frame_insert_head(fr, fb);
  
  // Передача данных
  bool res;
  while(true)
  {
    LED(D1, true);
    TIM_delay(100UL*Tmsec);
    LED(D1, false);
    res = RI_Send(fr);
    if (!res)
      LED(D2, true);
    else
      LED(D2, false);
    TIM_delay(900UL*Tmsec);
  };
  
  frame_delete(fr);
}

static void RadioRecvTest1(void)
{
  uint8_t DATA_RECV[10] = {0,1,2,3,4,5,6,7,8,9};
  uint8_t key[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  uint8_t nonce[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  
  NT_Init(); // Управление прериваниями таймера сна
  TIM_init();// Таймер MAC для измерения интервалов времени
  TIC_Init();// Контроллер интервалов времени (зависит от NT)
  RI_init(); // Иницилизация радио ( использует TIM и NT)
  AES_init();// Иницилизация шифровальщика
  
  RI_StreamCrypt(true);
  RI_SetChannel(CH11);
  RI_setKEY(key);
  RI_setIV(nonce);
  
  // Примем данных
  frame_s *fr;
  
  // Прием данных
  bool res;
  while(true)
  {
    fr = RI_Receive(2000);
    LED(D2, false);
    if (fr == NULL)
      continue;
    
    if (memory_compare((char*)DATA_RECV, fr->head->payload, 10))
      LED(D2, true);
    
    frame_delete(fr);
  };
  
  frame_delete(fr);
}

static void one_speed_test(frame_s *fr)
{
  bool res;
  TimeStamp_s start, stop, *ts0;
  do
  {
    TIM_TimeStamp(&start);
    res = RI_Send(fr);
    TIM_TimeStamp(&stop);
  } while (!res);
  
  uint32_t passed = TIM_passedTime(&start, &stop);
  uint16_t passed_sfd = fr->meta.TIMESTAMP;
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, 
      "Data size = %d. Full time = %lu us. SFD time = %d us\n"
      ,fr->head->len, passed, passed_sfd);
 
}
static void speed_test(void)
{
  uint8_t DATA_SEND[100];
  uint8_t key[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  uint8_t nonce[16] = {18,11,12,13,14,15,16,17,10,11,12,13,14,15,16,17};
  uint8_t size_tests[] = {10, 50, 100, 125};
  
  TIM_init();
  RI_init();
  AES_init();
  
  RI_StreamCrypt(true);
  RI_SetChannel(CH11);
  RI_setKEY(key);
  RI_setIV(nonce);
  
  // Подготовим данные к отправке
  
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "SEND SPEED TEST WITH DATA ENCODE\n");  
  for (uint8_t s = 0; s < sizeof(size_tests) ; s++)
  {
    frame_s *fr = frame_create();
    fbuf_s *fb = fbuf_create(FB_RAW_LAY, DATA_SEND, size_tests[s]);
    frame_insert_head(fr, fb);
    one_speed_test(fr);
    frame_delete(fr);
  }
  
  RI_StreamCrypt(false);
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "SEND SPEED TEST WITHOUT DATA ENCODE\n"); 
  for (uint8_t s = 0; s < sizeof(size_tests) ; s++)
  {
    frame_s *fr = frame_create();
    fbuf_s *fb = fbuf_create(FB_RAW_LAY, DATA_SEND, size_tests[s]);
    frame_insert_head(fr, fb);
    one_speed_test(fr);
    frame_delete(fr);
  }
  
  RI_StreamCrypt(true);
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "SEND IN SEND_TIME 1000 ticks AFTER TS0\n"); 
  for (uint8_t s = 0; s < sizeof(size_tests) ; s++)
  {
    frame_s *fr = frame_create();
    fbuf_s *fb = fbuf_create(FB_RAW_LAY, DATA_SEND, size_tests[s]);
    frame_insert_head(fr, fb);
    fr->meta.SEND_TIME = 1000;
    TIC_Init(); // При иницилизации TS0 отсчитываеться с этого момента
    one_speed_test(fr);
    frame_delete(fr);
  }  
  

  RI_StreamCrypt(true);
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "SEND IN SEND_TIME 1000 ticks AFTER TS0\n"); 
  for (uint8_t s = 0; s < sizeof(size_tests) ; s++)
  {
    frame_s *fr = frame_create();
    fbuf_s *fb = fbuf_create(FB_RAW_LAY, DATA_SEND, size_tests[s]);
    frame_insert_head(fr, fb);
    fr->meta.SEND_TIME = 1000;
    TIC_Init(); // При иницилизации TS0 отсчитываеться с этого момента
    one_speed_test(fr);
    frame_delete(fr);
  } 
  
  RI_StreamCrypt(true);
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "SEND IN SEND_TIME 1000 ticks AFTER TS0\n"); 
  for (uint8_t s = 0; s < sizeof(size_tests) ; s++)
  {
    frame_s *fr = frame_create();
    fbuf_s *fb = fbuf_create(FB_RAW_LAY, DATA_SEND, size_tests[s]);
    frame_insert_head(fr, fb);
    fr->meta.SEND_TIME = 1000;
    TIC_Init(); // При иницилизации TS0 отсчитываеться с этого момента
    one_speed_test(fr);
    frame_delete(fr);
  } 
  
}

// Тесты скорости
void suite_RADIO(void)
{
  umsg_line("RADIO module");
  speed_test();
}
// Для тестирования радио запрограмируй два узла. Первый узел RadioSendTest1();
// Второй узел RadioRecvTest1(); Первый будет раз в секунду передавать
// шифрованные данные. Второй при приеме и удачном декодировании включит
// светодиод.
void suite_RADIO_HW(void)
{
  //umsg_line("NTMR module");
  //RadioSendTest1();
  //
  RadioRecvTest1();
}
#include "utest_suite.h"
#include "MAC.h"
#include "frame.h"
#include "delays.h"
#include "nwdebuger.h"

static void test_ACK_SEND(void)
{
  MAC_Init();
  
  uint8_t DATA_SEND[10] = {0,1,2,3,4,5,6,7,8,9};
  TimeStamp_s start, stop;
while(1)
{
  // Подготовим данные к отправке
  frame_s *fr = frame_create();
  fbuf_s *fb = fbuf_create(FB_RAW_LAY, DATA_SEND, sizeof(DATA_SEND));
  fr->meta.SEND_TIME = 0;
  fr->meta.CH = CH11;
  frame_insert_head(fr, fb);
   
  // Нам не важно что структура не соотетсвует ACK. Важно чтоб пакет ушел
  TIM_TimeStamp(&start);
  MAC_ACK_Send(fr);
  TIM_TimeStamp(&stop);
  uint32_t passed = TIM_passedTime(&start, &stop);
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, "MAC_ACK_Send time = %lu\n", passed);
}
}

void suite_MAC(void)
{
  test_ACK_SEND();
}
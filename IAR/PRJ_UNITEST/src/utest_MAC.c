#include "utest_suite.h"
#include "MAC.h"
#include "frame.h"
#include "delays.h"
#include "nwdebuger.h"
#include "ioCC2530.h"

static void test_ACK_SEND(void)
{
  MAC_Init();
  
  bool res;
  uint8_t DATA_SEND[10] = {0,1,2,3,4,5,6,7,8,9};
  TimeStamp_s start, stop;
while(1)
{
  // Подготовим данные к отправке
  frame_s *fr = frame_create();
  frame_addHeader(fr, DATA_SEND, sizeof(DATA_SEND));
  fr->meta.SEND_TIME = 0;
  fr->meta.CH = CH11;
   
  // Нам не важно что структура не соотетсвует ACK. Важно чтоб пакет ушел
  TIM_TimeStamp(&start);
  res = MAC_ACK_Send(fr);
  TIM_TimeStamp(&stop);
  uint32_t passed = TIM_passedTime(&start, &stop);
  LOG(MSG_ON | MSG_INFO | MSG_TRACE, 
      "MAC_ACK_Send time = %lu. Success %d \n", passed, res);
}
}

static void stub_se(uint8_t ts)
{
}

static void test_TS_Send(void)
{
  MAC_Init();
  TIC_SetSECallback(stub_se);
  EA = 1;
  uint8_t DATA_SEND[10] = {0,1,2,3,4,5,6,7,8,9};  
  uint8_t TS = 5, attempts = 3;
  
  frame_s *fr; 
  
  while (true)
  {
    if (!MAC_GetTXState(TS))
    {
      fr = frame_create();
      frame_addHeader(fr, DATA_SEND, sizeof(DATA_SEND));
      fr->meta.SEND_TIME = 0;
      fr->meta.TS = TS;
      fr->meta.CH = CH11;
      MAC_Send(fr, attempts);
      LOG(MSG_ON | MSG_INFO | MSG_TRACE, "Frame puted to MAC layer at %lu sec\n", TIC_GetRTC());
    }
  }
}

void suite_MAC(void)
{
  // test_TS_Send отсылает пакеты по расписанию тайм слотов
  // Нужно запрограмировать второе устройство RadioRecvTest1
  // тогда будет гореть светодиод при приеме сигнала
  test_TS_Send(); 
  test_ACK_SEND();
}
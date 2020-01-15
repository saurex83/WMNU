#include "manager.h"
#include "coder.h"
#include "delays.h"
#include "ethernet.h"
#include "frame.h"
#include "LLC.h"
#include "MAC.h"
#include "NTMR.h"
#include "RADIO.h"
#include "TIC.h"
#include "sync.h"
#include "config.h"
#include "frame.h"
#include "nwdebuger.h"
#include "basic.h"
#include "config.h"
#include "ioCC2530.h"

static void MG_Init();
static void MG_Reset();
static void re_start();
static bool network_discovery();

static bool MG_MODULES_INITED = false; //!< Были ли запущенны аппаратные модули

// Публичные методы
bool MG_Connect();

// Переменные модуля


static void MG_Init()
{
  CF_init();
  NT_Init();
  TIM_init();
  TIC_Init();
  RI_init();
  AES_init();
  MAC_Init();
  SY_Init();
  LLC_Init();
  // TODO Необходимо очищать все ранее выделеную память malloc
}

static void MG_Reset()
{   
  TIC_Reset();
  MAC_Reset();
  SY_Reset();
  LLC_Reset();
}

static void re_start()
{
    if (!MG_MODULES_INITED)
  {
    MG_Init();
    MG_MODULES_INITED = true;
  }
  else
    MG_Reset(); 
    
  CF_init();
  MAC_setIV(CONFIG.stream_iv);
  MAC_setKEY(CONFIG.stream_key);
  SY_setIV(CONFIG.stream_iv);
  SY_setKEY(CONFIG.stream_key);
}

/**
@brief Алгоритм подключения к сети
*/
static bool network_discovery()
{
  uint16_t panid;
  bool con = false;
  
  // Тупой алгоритм поиска сети.
  while (!con)
  {
    con = SY_SYNC_NETWORK(&panid, 5000);
  }
  return con;
}

/**
@brief Создание сети в режиме шлюза
*/
static bool master_mode()
{
  // Включает обработку начала слота TS1 для программы собранной с ключом 
  // GATEWAY. MAC_TS1_HNDL_MASTER будет создавать пакеты синхронизации.
  MAC_Enable(true);
  SY_Enable(true);
  return true;
}

/**
@brief Создание сети в режиме узла
*/
static bool slave_mode()
{
  // В режиме slave сначало нужно подключисться к сети
  network_discovery();
  
  // После синхронизации с сетью разрешаем обработку временных слотов
  // Программа собрана без ключа GATEWAY и активной функцией обработки TS1
  // будет MAC_TS1_HNDL_SLAVE. Она отвечает за поддержание синхронизации
  // и периодиескую ректрансляцию пакетов.
  
  MAC_Enable(true);
  SY_Enable(true);
  return true;
}

bool MG_Connect()
{
  EA = 0;
  re_start();
  EA = 1;
  if (frame_getCount())
  {
    LOG_ON("Frame_s count: %d \r\n",frame_getCount());
    return false;
  }
  
#ifdef GATEWAY
    bool con = master_mode();
#else
    bool con = slave_mode();  
#endif
    
    return con;
}
 
  

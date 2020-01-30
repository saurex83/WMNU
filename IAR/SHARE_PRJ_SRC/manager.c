#include "manager.h"
#include "coder.h"
#include "delays.h"
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
#include "dma.h"
#include "rx_buff.h"

static void MG_Init();

static bool MG_MODULES_INITED = false; //!< Были ли запущенны аппаратные модули
static bool NETWORK_SEED = false; //!< Раздача сети шлюзом

// Публичные методы
void neocore_hw_init();
bool neocore_stack_reset();
void network_seed_enable(bool en);
bool network_discovery(uint8_t timeout_ms);
bool MG_Connect();

// Переменные модуля

/**
@brief Начальная иницилизация стека
*/
static void MG_Init()
{
  NETWORK_SEED = false;
  DMA_init();
  CF_init();
  NT_Init();
  TIM_init();
  TIC_Init();
  RI_init();
  AES_init();
  MAC_Init();
  SY_Init();
  LLC_Init();
  
#ifdef GATEWAY
  RXB_Init();
#endif
  
  // Загрузка дефолтных параметров
  MAC_setIV(CONFIG.stream_iv);
  MAC_setKEY(CONFIG.stream_key);
  SY_setIV(CONFIG.stream_iv);
  SY_setKEY(CONFIG.stream_key);  
  // TODO Необходимо очищать все ранее выделеную память malloc
}

/**
@brief Иницилизация аппаратуры стека
*/
void neocore_hw_init()
{
  if (!MG_MODULES_INITED){
    MG_Init();
    MG_MODULES_INITED = true;
  }  
}

/**
@brief Сброс всех настроек стека и удаление пакетов
*/
bool neocore_stack_reset(){
  TIC_Reset();
  MAC_Reset();
  SY_Reset();
  LLC_Reset();

#ifdef GATEWAY
  RXB_Reset();
#endif

  network_seed_enable(false);
  
  // Проверим все ли пакеты уничтожены
  uint8_t nbrF = frame_getCount(); 
  ASSERT(nbrF == 0);
  if (!nbrF)
    return true;
  return false;
}

/**
@brief Алгоритм подключения к сети
@param[in] timeout_ms время поиска сети
*/
bool network_discovery(uint8_t timeout_ms)
{
  uint16_t panid;
  bool con = false;
  
  // Тупой алгоритм поиска сети.
  while (!con)
  {
    con = SY_SYNC_NETWORK(&panid, timeout_ms);
  }
  
  if (con){ // После синх. разрешаем обработку слотов
    NETWORK_SEED = true;
    MAC_Enable(true);
    SY_Enable(true);
  }
  return con;
}

/**
@brief Начинаем раздачу сети
@param[in] en true начать раздачу сети
*/
void network_seed_enable(bool en)
{
  if (en){
    
    MAC_setIV(CONFIG.stream_iv);
    MAC_setKEY(CONFIG.stream_key);
    SY_setIV(CONFIG.stream_iv);
    SY_setKEY(CONFIG.stream_key);    
    
    NETWORK_SEED = true;
    MAC_Enable(true);
    SY_Enable(true);
  }
  else{ 
    NETWORK_SEED = false;
    MAC_Enable(false);
    SY_Enable(false);
  }
}

/**
@brief Возвращает состояние сети
@return true если сеть раздается
*/
bool get_network_seed_status(void){
  return NETWORK_SEED;
}

 
  

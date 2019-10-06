#include "RADIO.h"
#include "stdlib.h"
#include "nwdebuger.h"
#include "ioCC2530.h"

// Публичные методы
RI_s* RI_create(void);
bool RI_delete(RI_s *ri);

// Методы класса
static void RI_On(void);
static void RI_Off(void);
static bool RI_SetChannel(uint8_t CH);
static bool RI_Send(FChain_s *fc);
static FChain_s* RI_Receive(uint8_t timeout);
static uint32_t RI_GetCRCError(void);
static uint32_t RI_GetCCAReject(void);
static uint32_t RI_GetUptime(void);
static void RI_StreamCrypt(bool state);

// Приватные методы
static void random_core_init(void);

// TODO можно добавть простой алгоритм перестановки для сокрытия данных 
// при передаче RAW формата. Алгоритм должен быть достаточно простой и 
// перестанавливать биты, смешивать их с ключом. Алгорим не производит 
// контроль дешифровки с помощью цифровых подписей.
// Шифрование можно включаеть/отключаеть
// Возможно попробовать аппаратный модуль, так как к уже зашифрованому пакету
// IP добавятся данные и зашифруются еще одним ключем, надежность возрастет.
// Думаю это не будет являтся двойным шифрованием и будет устойчиво к взлому
static void RI_BitRawCrypt(uint8_t *src, uint8_t size); // Шифрование передачи
static void RI_BitRawDecrypt(uint8_t *src, uint8_t size); // Дешифрока приема

// Переменные модуля
bool RADIO_CREATED = false;
uint32_t RI_UPTIME = 0; // Время работы радио в мс. 49 дней максимум
uint32_t RI_CRC_ERROR = 0; // Количество ошибок CRC
uint32_t RI_CCA_REJECT = 0; // Количество отказов передач из-за CCA

enum TX_POWER_e 
{
  // +4.5дБ = p4x5, -1.5дБ = m1x5.
  // +4.5 = 34мА, +1 = 29мА, -4 = 26мА, -10 = 25 мА, -22 = 23 мА
  p4x5 = 0xF5, p2x5 = 0xE5, p1x0 = 0xD5, m0x5 = 0xC5, m1x5 = 0xB5, m3x0 = 0xA5,
  m4x0 = 0x95, m6x0 = 0x85, m8x0 = 0x75, m10x0 = 0x65, m12x0 = 0x55, 
  m14x0 = 0x45, m16x0 = 0x35, m20x0 = 0x15, m22x0 = 0x05
};

#define IEEE_MODE 0     // Режимы фазы сигнала
#define NOT_IEEE_MODE 1 // Режимы фазы сигнала
#define RSSI_OFFSET -76 // Константа для вычисления реальной мощности

// Расчет качества связи. Коэффициенты получаются эксперементально.
// Есть предположение, что количество ошибок будет пропорционально квадрату
// коеффициента корреляции сигнала.
// LIQ = 255 передача 1000 пакетов без ошибок CRC
// LIQ = 127 передача 1000 пакетов с 50% ошибок CRC
// LIQ = 0   передача 1000 пакетов с 100% ошибок CRC 
#define CONST_A (int8_t)0
#define CONST_B (int8_t)1
#define LIQ_CALC(corr) ((corr-CONST_A)*CONST_B)

struct
{
  uint8_t CH;       // Номер канала с 11 до 26 включительно
  uint8_t TX_POWER; // Мощность радиопередатчика по умолчанию. 7 бит
  bool MODULATION_MODE; // false - совместимость с IEEE 802.15.4
  bool STREAM_CRYPT_ENABLE; // Шифрование выходного потока данных
} RADIO_CFG;

typedef struct // Структура двух байт FCS1,FCS2 при APPEND_DATA_MODE = 0
{
  int8_t rssi;
  int8_t correlation:7;
  uint8_t CRC_OK:1;
} __attribute__((packed)) FCS_t;


RI_s* RI_create(void)
{
  
  if (RADIO_CREATED)
  {
    ASSERT_HALT(RADIO_CREATED == false, "RADIO already created");
    return NULL;
  }
  
  RI_s* ri = malloc(RI_S_SIZE);
  ASSERT_HALT(ri != NULL, "Memory allocation fails");
  
  if (ri == NULL)
    return NULL;
  
  // Заполняем структуру указателей
  ri->RI_On = RI_On;
  ri->RI_Off = RI_Off;
  ri->RI_SetChannel = RI_SetChannel;
  ri->RI_Send = RI_Send;
  ri->RI_Receive = RI_Receive;
  ri->RI_GetCRCError = RI_GetCRCError;
  ri->RI_GetCCAReject = RI_GetCCAReject;
  ri->RI_GetUptime = RI_GetUptime;
  ri->RI_StreamCrypt = RI_StreamCrypt;

  // Настройки поумолчанию
  RADIO_CFG.CH = CH11;
  RADIO_CFG.TX_POWER = m0x5;
  RADIO_CFG.MODULATION_MODE = IEEE_MODE;
  
  // Пост действия с радио
  random_core_init();
  return ri;
}

bool RI_delete(RI_s *ri)
{
  free(ri);
  RADIO_CREATED = false;
  return true;
}

static  void RI_On(void)
{
/*
APPEND_DATA_MODE = 0. FCS1 = signed rssi, FCS2.7 CRC OK, FCS2.6-0 Correlation
A correlation value of ~110 indicates a maximum quality frame while a value 
of ~50 is typically the lowest quality frames detectable by CC2520. 
*/
/* После включения радио находится в активном режиме но приемник выключен */
  T1CCTL0=5;
}

static void RI_Off(void)
{
}

static bool RI_SetChannel(uint8_t CH)
{
  if ((CH >=11) && (CH<=26))
  {
    RADIO_CFG.CH = CH;
    return true;
  }
  return false;
}

static bool RI_Send(FChain_s *fc)
{
  return 0;
}

static FChain_s* RI_Receive(uint8_t timeout)
{
}

static uint32_t RI_GetCRCError(void)
{
  return RI_CRC_ERROR;
}

static uint32_t RI_GetCCAReject(void)
{
  return RI_CCA_REJECT;
}

static uint32_t RI_GetUptime(void)
{
  return RI_UPTIME;
}

static void random_core_init(void)  
{
  // TODO получение случайный чисел из радио эфира eth_seed.
  // Хорошо будет если количество единиц и нулей в eth_seed будет одинаково.
  // и они будут хорошо перемешенны.
  unsigned int rnd_core = 0x3212;
  srand(rnd_core);
}

static void RI_StreamCrypt(bool state)
{
  RADIO_CFG.STREAM_CRYPT_ENABLE = state;
}

static void RI_BitRawCrypt(uint8_t *src, uint8_t size)
{
  if (!RADIO_CFG.STREAM_CRYPT_ENABLE)
    return;
}

static void RI_BitRawDecrypt(uint8_t *src, uint8_t size)
{
   if (!RADIO_CFG.STREAM_CRYPT_ENABLE)
    return;
}
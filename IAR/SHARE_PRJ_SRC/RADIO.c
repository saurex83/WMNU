/*!
\file 
\brief Модуль содержит реализацию интерфейса радио для приема/передачи данных.
*/

#include "RADIO.h"
#include "stdlib.h"
#include "nwdebuger.h"
#include "ioCC2530.h"
#include "Radio_defs.h"

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
static inline void OP_EXE(uint8_t op);

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
static inline void setFreq(uint8_t CH); // Установка частоты передатчика
static void LoadTXData(uint8_t *src, uint8_t len); // Загрузка данных в tx buf

// Переменные модуля
bool RADIO_CREATED = false;
uint32_t RI_UPTIME = 0; // Время работы радио в мс. 49 дней максимум
uint32_t RI_CRC_ERROR = 0; // Количество ошибок CRC
uint32_t RI_CCA_REJECT = 0; // Количество отказов передач из-за CCA

/*!
\brief Константы для установки выходной мощности радиопередатчика.
Пример: p4x5 = +4,5 дБ. m0x5 = -0.5 дБ
*/
enum TX_POWER_e 
{
  // +4.5дБ = p4x5, -1.5дБ = m1x5.
  // +4.5 = 34мА, +1 = 29мА, -4 = 26мА, -10 = 25 мА, -22 = 23 мА
  p4x5 = 0xF5, p2x5 = 0xE5, p1x0 = 0xD5, m0x5 = 0xC5, m1x5 = 0xB5, m3x0 = 0xA5,
  m4x0 = 0x95, m6x0 = 0x85, m8x0 = 0x75, m10x0 = 0x65, m12x0 = 0x55, 
  m14x0 = 0x45, m16x0 = 0x35, m20x0 = 0x15, m22x0 = 0x05
};


#define IEEE_MODE 0     //!< Режимы фазы сигнала
#define NOT_IEEE_MODE 1 //!< Режимы фазы сигнала
#define RSSI_OFFSET -76 //!< Константа для вычисления реальной мощности

/*!
\details 
 Расчет качества связи. Коэффициенты получаются эксперементально.
 Есть предположение, что количество ошибок будет пропорционально квадрату
 коеффициента корреляции сигнала.
 LIQ = 255 передача 1000 пакетов без ошибок CRC
 LIQ = 127 передача 1000 пакетов с 50% ошибок CRC
 LIQ = 0   передача 1000 пакетов с 100% ошибок CRC
*/ 
#define CONST_A (int8_t)0 //!< Коэффициент A
#define CONST_B (int8_t)1 //!< Коэффициент B
#define LIQ_CALC(corr) ((corr-CONST_A)*CONST_B) //!< Формула вычисление LIQ 
#define FRQ_CALC(x) (11+5*(x-11)) //!< Формул вычисления частоты сигнала

/// Глобальные параметры модуля
struct
{
  uint8_t CH;       //!< Номер канала с 11 до 26 включительно
  uint8_t TX_POWER; //!< Мощность радиопередатчика по умолчанию. 7 бит
  bool MODULATION_MODE; //!< false - совместимость с IEEE 802.15.4
  bool STREAM_CRYPT_ENABLE; //!< Шифрование выходного потока данных
} RADIO_CFG;

typedef struct // Структура двух байт FCS1,FCS2 при APPEND_DATA_MODE = 0
{
  int8_t rssi;
  int8_t correlation:7;
  uint8_t CRC_OK:1;
} __attribute__((packed)) FCS_t;

/*!
\brief Создает обьект с интерфейсом радио. Можно создать только один обьект.
\return Возвращает указатель на обьект.
*/
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
  RADIO_CFG.STREAM_CRYPT_ENABLE = true;
  // Пост действия с радио
  random_core_init();
  return ri;
}

/*!
\brief Удаляеть обьект радио
\param[in] *ri Указатель на удаляемый обьект
\return Возвращает true в случаии успешного удаления 
*/
bool RI_delete(RI_s *ri)
{
  free(ri);
  RADIO_CREATED = false;
  return true;
}

/*!
\brief Переводит радио в активный режим и устанавливает параметры.
*/
static void RI_On(void)
{
/*
APPEND_DATA_MODE = 0. FCS1 = signed rssi, FCS2.7 CRC OK, FCS2.6-0 Correlation
A correlation value of ~110 indicates a maximum quality frame while a value 
of ~50 is typically the lowest quality frames detectable by CC2520. 
*/
/* После включения радио находится в активном режиме но приемник выключен */
  //T1CCTL0=5;
  
  // Устанавливаем частоту радиопередатчика
  setFreq(RADIO_CFG.CH);
  
  // Устанавливаем мощность выходного сигнала
  TXPOWER = RADIO_CFG.TX_POWER;
  
  // Устанавливаем режим модуляции
  MDMTEST1_u MDM1;
  MDM1.value = MDMTEST1;
  MDM1.bits.MODULATION_MODE = RADIO_CFG.MODULATION_MODE;
  MDMTEST1 = MDM1.value;
}

/*!
\brief Выполняет команду на командном процессоре микросхемы радиопередатчика
\param[in] op Номер OP команды см. документацию
*/
static inline void OP_EXE(uint8_t op) 
{
  while (RFST); // Убедимся что радио не выполняет команд
  RFST = op;
  while (RFST); // Дождемся завершения
}

/*!
\brief Отключает радиопередатчик
*/
static void RI_Off(void)
{
  OP_EXE(OP_SRFOFF); // Останавливаем прием и передачу. отключаем синтезатор
}

/*!
\brief Устанавливает канал радиопередатчика.
\param[in] CH Номера каналов [11..26]
\return Возвращает true если аргументы верны
*/
static bool RI_SetChannel(uint8_t CH)
{
  if ((CH >=11) && (CH<=26))
  {
    RADIO_CFG.CH = CH;
    return true;
  }
  return false;
}

/*!
\brief Передает данные в эфир
\details Функция может самостоятельно шифровать поток данных, выполнять
 обратные вызовы при передаче. Увеличивает RI_CCA_REJECT
\param[in] *fc Указатель на структуру FChain, содержащию данные для передачи
\return Возвращает true в случаи успешной передачи
*/
static bool RI_Send(FChain_s *fc)
{
  ASSERT_HALT(fc != NULL, "fc NULL");
  if (fc == NULL)
    return false;
  
  // Устанавливаем частоту передачи пакета
  setFreq(fc->meta.CH);
  
  // TODO 
  /*
1. Извлекаем из fc данные в массив
2. Если нужно массив шифруем
3. Загружеаем данные в tx buf
4. Пробуем передать с STXONCCA

Другой вариант
1. Очищаем буфер
2. Пробуем STXONCCA
3. Если канал доступен вызываем callback
4. Извлекаем данные из fc
5. шифруем данные
6. загружаем данные

*/
  
  // Загружаем данные из пакета в буфер передачи
  LoadTXData(NULL, 0);
  
  
  OP_EXE(OP_STXONCCA);
  
  
  return true;
}

static void LoadTXData(uint8_t *src, uint8_t len)
{
  // Очищаем буфер передатчика
  OP_EXE(OP_SFLUSHTX); 
  // 
  while (len)
  {
    RFD = *src;
    src++;
    len--;
  }
};
static inline void setFreq(uint8_t CH)
{
  ASSERT_HALT( (CH >= 11) && (CH <= 26), "Incorrect radio channel");
  // Устанавливаем частоту радиопередатчика
  FREQCTRL_u FRQ;
  FRQ.value = FRQ_CALC(RADIO_CFG.CH);
  FREQCTRL = FRQ.value;  
}

/*!
\brief Принимает данные из эфира
\details Функция принимает данные из эфира. Проводит проверку CRC, увеличивает
RI_CRC_ERROR. Дешифрует данные при необходимости. Отмечает время прихода SFD.
\param[in] timeout Время ожидания данных в милисекундах
\return Возвращает NULL если данных нет
*/
static FChain_s* RI_Receive(uint8_t timeout)
{
  OP_EXE(OP_SRXON);
  return NULL;
}

/*!
\brief Возвращает количество ошибок возникших с момента иницилизации
\return Количество CRC ошибок
*/
static uint32_t RI_GetCRCError(void)
{
  return RI_CRC_ERROR;
}

/*!
\brief Возвращает количестов отказов передачи пакета из-за занятости канала.
\return Количество отказов CCA
*/
static uint32_t RI_GetCCAReject(void)
{
  return RI_CCA_REJECT;
}

/*!
\brief Возвращает суммарное время работы радио в режиме прием/передача
\return Вермя в милисекундах
*/
static uint32_t RI_GetUptime(void)
{
  return RI_UPTIME;
}

/*!
\brief Иницилизирует ядро генератора случайных чисел данными из эфира
*/
static void random_core_init(void)  
{
  // TODO получение случайный чисел из радио эфира eth_seed.
  // Хорошо будет если количество единиц и нулей в eth_seed будет одинаково.
  // и они будут хорошо перемешенны.
  unsigned int rnd_core = 0x3212;
  srand(rnd_core);
}

/*!
\brief Установка разрешения шифрования потока данных
\param[in] true - включить шифрование
*/
static void RI_StreamCrypt(bool state)
{
  RADIO_CFG.STREAM_CRYPT_ENABLE = state;
}

/*!
\brief Шифрует область памяти если шифрование разрешено
\param[in,out] *src Указатель на начало области шифрования
\param[in] size Размер шифруемых данных
*/
static void RI_BitRawCrypt(uint8_t *src, uint8_t size)
{
  if (!RADIO_CFG.STREAM_CRYPT_ENABLE)
    return;
}

/*!
\brief Расшифровка область памяти если шифрование разрешено
\param[in,out] *src Указатель на начало области дешифрования
\param[in] size Размер расшифруемых данных
*/
static void RI_BitRawDecrypt(uint8_t *src, uint8_t size)
{
   if (!RADIO_CFG.STREAM_CRYPT_ENABLE)
    return;
}
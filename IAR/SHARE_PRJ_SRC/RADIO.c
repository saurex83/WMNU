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
static bool SendWithoutCallbalck(FChain_s *fc);
static bool SendWithCallback(FChain_s *fc);

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
  op |= 0xE0;
  
  if (RFST == 0xD0)
  {
    RFST = op;
  }
  else
  {
    while (RFST); // Убедимся что радио не выполняет команд
    RFST = op;
  }
  while (RFST); // Дождемся завершения
}

/*!
\brief Отключает радиопередатчик
*/
static void RI_Off(void)
{
  OP_EXE(OP_SRFOFF); // Останавливаем прием и передачу. отключаем синтезатор
  // На диаграмме состояния передачтчика это режим idle
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
  
  // Убедимся, что первый элемент данных это ETH_H
  FC_iteratorToHead(fc);
  FItem_s* fi = FC_getIterator(fc);
  FItem_t ft = FI_getType(fi);
  ASSERT_HALT(ft != ETH_H , "Incorrect fchain");
  
  // Устанавливаем частоту передачи пакета
  setFreq(fc->meta.CH);
  
  bool send_res; // Результат передачи данных
  
  // Отправка данных с обратным вызовом или нет
  if (fc->meta.SFD_Callback == NULL)
    send_res = SendWithoutCallbalck(fc);
  else
    send_res = SendWithCallback(fc);
  
  // В случаи ошибки передачи увеличиваем счетчик RI_CCA_REJECT
  if (!send_res)
    RI_CCA_REJECT++;
   
  //TODO Измерением времени которое требуется на передачу данных
  // Учет времени работы передатчика
    
  return send_res;
}

/**
@brief Отправка сообщения без обратного вызова
@return true в случаи успеха
*/
static bool SendWithoutCallbalck(FChain_s *fc)
{
  uint8_t *data = malloc(128); // Выделяем область данных под сообщение
  uint8_t data_size; // Размер данных в буфере
  
  bool result = true;
  switch(true)
  {
    case true:
      ASSERT_HALT(data != NULL, "Cant memory allocate");
      FC_copyChainData(fc, data, &data_size); // Копируем данные в буфер
      // Шифруем данные при необходимости
      RI_BitRawCrypt(data, data_size);
      // Копируем данные в буфер. Очистка буфера автоматическая
      LoadTXData(data, data_size);
      // Для начала передачи по команде STXONCCA нужно включить приемник
      OP_EXE(OP_SRXON);
      // Ждем пока статус RSSI_VALID станет true
      while(!RSSISTAT);
      // Очищаем флаг передачи сообщения
      RFIRQF1 &= ~RFIRQF1_TXDONE;
      // Начинаем передачу данных
      OP_EXE(OP_STXONCCA);
      // Произошла ошибка передачи если SAMPLED_CCA false
      FSMSTAT1_u fsmstat1;
      fsmstat1.value = FSMSTAT1;
      if (!fsmstat1.bits.SAMPLED_CCA)
      {
        result = false;
        break;
      }
      
      // Ждем пока завершиться передача пакета
      while (fsmstat1.bits.TX_ACTIVE)
          fsmstat1.value = FSMSTAT1;
      
        // Проверим переданно ли сообщение TX_FRM_DONE
      if (!(RFIRQF1 & RFIRQF1_TXDONE))
      {
        result = false;
        break;
      }
  }

  free(data);
  OP_EXE(OP_SRFOFF);
  if (result)
    return true;
  return false; 
}

/**
@brief Отправка сообщения с обратным вызовом
@return true в случаи успеха
*/
static bool SendWithCallback(FChain_s *fc)
{
  uint8_t *data = malloc(128); // Выделяем область данных под сообщение
  uint8_t data_size; // Размер данных в буфере
  
  bool result = true;
  switch (true)
  {
    case true:
      ASSERT_HALT(data != NULL, "Cant memory allocate");
        
      // Для начала передачи по команде STXONCCA нужно включить приемник
      OP_EXE(OP_SRXON);
      
      // Ждем пока статус RSSI_VALID станет true
      while(!RSSISTAT);
      
      // Очищаем флаг передачи сообщения и передачи SFD
      RFIRQF1 &= ~RFIRQF1_TXDONE;
      RFIRQF0 &= ~RFIRQF0_SFD;  
      
      // Начинаем передачу данных
      OP_EXE(OP_STXONCCA);
      
      // Произошла ошибка передачи если SAMPLED_CCA false
      FSMSTAT1_u fsmstat1;
      fsmstat1.value = FSMSTAT1;
      
      if (!fsmstat1.bits.SAMPLED_CCA)
      {
        result = false;
        break;
      }
      
      // TODO После SFD начинаются данные. Нужно вызывать callback после 
      // SAMPLED_CCA и TX_STATE.
      // Ждем завершения передачи SFD
     // while (!RFIRQF0 & RFIRQF0_SFD);
    
      // Ждем начала передачи преамбалы
      while (fsmstat1.bits.TX_ACTIVE)
          fsmstat1.value = FSMSTAT1;
      
      // Как только началась передача преамбалы запрашиваем fchain
      FChain_s *new_fc = fc->meta.SFD_Callback();
      FC_copyChainData(new_fc, data, &data_size); // Копируем данные в буфер
      // Шифруем данные при необходимости
      RI_BitRawCrypt(data, data_size);
      // Копируем данные в буфер. Очистка буфера автоматическая
      LoadTXData(data, data_size);  
      
      // Ждем пока завершиться передача пакета
      while (fsmstat1.bits.TX_ACTIVE)
          fsmstat1.value = FSMSTAT1;
      
      // Проверим переданно ли сообщение TX_FRM_DONE
      if (!(RFIRQF1 & RFIRQF1_TXDONE))
      {
        result = false;
        break;
      }
  }
  free(data);
  OP_EXE(OP_SRFOFF);
  if (result)
    return true;
  return false; 
}

static void LoadTXData(uint8_t *src, uint8_t len)
{
  uint8_t *src_ptr = src;
  // Очищаем буфер передатчика
  OP_EXE(OP_SFLUSHTX); 
  // 
  while (len)
  {
    RFD = *src_ptr;
    src_ptr++;
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

/**
@breif Подссчет количества бит в байте
@return Возвращает количество бит
*/
static uint8_t bits_count(uint8_t value) {
  int ret = 0;
  for ( ; value; value >> 1 )
    ret += value & 1;
  return ret;
}

/**
@brief Возвращает случайное число с числом включеных бит от 2 до 6 включительно
@details Случайные числа появляются в регистре с частотой 170нс. Функция 
возвращает гарантированно новое значение. 
@return возвращает случайное число.
*/
static inline uint8_t readRandom(void)
{
  uint8_t rnd_val = 0;
  uint8_t bit_cnt = 0;
  while ( bit_cnt >= 2 && bit_cnt <= 6 )
  {
    rnd_val = RFRND;
    while (rnd_val == RFRND);
    rnd_val = RFRND;
    bit_cnt = bits_count(rnd_val);
  }
  return rnd_val;
}

/*!
\brief Иницилизирует ядро генератора случайных чисел данными из эфира
*/
static void random_core_init(void)  
{
  unsigned int rnd_core = 0;;
     
  RI_On();
  FREQCTRL = 0x00;
  CSPCTRL = 0x01;
  // TODO По какой то причине OP_EXE не выполняет команду.
  // регистра RFST читается как 0xD0. это его состояние при reset
  // Включаем демодулятор
  OP_EXE(OP_ISSTART);
  
  // Ждем пока статус RSSI_VALID станет true
  while(!RSSISTAT);
  
  // Ждем случайных чисел
  while (RFRND == 0);

  // Настраиваем ядро случайного генератора
  rnd_core = readRandom();
  rnd_core |= (unsigned int)readRandom()<<8;
  srand(rnd_core);
  
  // Включаем демодулятор
  OP_EXE(OP_CMD_SHUTDOWN);
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
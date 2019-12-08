/*!
\file 
\brief Модуль содержит реализацию интерфейса радио для приема/передачи данных.
*/

#include "RADIO.h"
#include "mem.h"
#include "nwdebuger.h"
#include "Radio_defs.h"
#include "TIC.h"
#include "delays.h"
#include "frame.h"
#include "string.h"
#include "coder.h"
#include "NTMR.h"

// Открытые методы модуля
void RI_init(void);
bool RI_SetChannel(uint8_t CH);
bool RI_Send(frame_s *fr);
frame_s* RI_Receive(uint16_t timeout);
uint32_t RI_GetCRCError(void);
uint32_t RI_GetCCAReject(void);
float RI_GetUptime(void);
void RI_StreamCrypt(bool state);
void RI_setKEY(void* ptr_KEY);
void RI_setIV(void* ptr_IV);

// Приватные методы
static void random_core_init(void);
static void RI_cfg(void);

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
static void UnLoadRXData(uint8_t *src, uint8_t len);
static bool SendData(frame_s *fc);
static bool RecvData(uint32_t timeout_us, uint16_t *SFD_TimeStamp);

// Переменные модуля
static float RI_UPTIME = 0; // Время работы радио в мс. 49 дней максимум
static uint32_t RI_CRC_ERROR = 0; // Количество ошибок CRC
static uint32_t RI_CCA_REJECT = 0; // Количество отказов передач из-за CCA
static uint8_t IV[16];  // Вектор иницилизации для кодирования
static uint8_t KEY[16]; // Ключ для кодирования

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

/**
@brief Установить вектор иницилизации для шифрования
@param[in] ptr_IV указатель на 16 байтный вектор иницилизации
*/
void RI_setIV(void* ptr_IV)
{
  memcpy(IV, ptr_IV, 16);
}

/**
@brief Установить ключ шифрования
@param[in] ptr_KEY указатель на 16 байтный ключ
*/
void RI_setKEY(void* ptr_KEY)
{
  memcpy(KEY, ptr_KEY, 16);
}

/*!
\brief Иницилизация радио интерфейса
*/
void RI_init(void)
{
  // Настройки поумолчанию
  RADIO_CFG.CH = CH11;
  RADIO_CFG.TX_POWER = m0x5;
  RADIO_CFG.MODULATION_MODE = IEEE_MODE;
  RADIO_CFG.STREAM_CRYPT_ENABLE = true;
  // Пост действия с радио
  random_core_init();
}

/*!
\brief Переводит радио в активный режим и устанавливает параметры.
*/
static void RI_cfg(void)
{
/*
APPEND_DATA_MODE = 0. FCS1 = signed rssi, FCS2.7 CRC OK, FCS2.6-0 Correlation
A correlation value of ~110 indicates a maximum quality frame while a value 
of ~50 is typically the lowest quality frames detectable by CC2520. 
*/
/* После включения радио находится в активном режиме но приемник выключен */
  
  // Устанавливаем частоту радиопередатчика
  setFreq(RADIO_CFG.CH);
  
  // Устанавливаем мощность выходного сигнала
  TXPOWER = RADIO_CFG.TX_POWER;
 
  FRMFILT0 = 0x00; // Отключаем фильтрацию пакетов
  
  // Устанавливаем режим модуляции
  MDMTEST1_u MDM1;
  MDM1.value = MDMTEST1;
  MDM1.bits.MODULATION_MODE = RADIO_CFG.MODULATION_MODE;
  MDMTEST1 = MDM1.value; 
}

/*!
\brief Устанавливает канал радиопередатчика.
\param[in] CH Номера каналов [11..26]
\return Возвращает true если аргументы верны
*/
bool RI_SetChannel(uint8_t CH)
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
\details Функция может самостоятельно шифровать поток данных, увеличивает 
 RI_CCA_REJECT при невозможности отправки сообщения, расчитывает время работы 
 радио передатчика. Отправка сообщения в заданное сетевое время 
 fr->meta.SEND_TIME. Если равно 0, то отправка через случайноу время в мкс.
 Данные автоматически оборачиваются в LEN, CRC1,CRC2
\param[in] *fr Указатель на структуру frame_s RAW_LAY
\return Возвращает true в случаи успешной передачи
*/
bool RI_Send(frame_s *fr)
{
  ASSERT_HALT(fr != NULL, "fr NULL");
  
  // Устанавливаем частоту передачи пакета
  RI_cfg();
  
  bool send_res; // Результат передачи данных
  TimeStamp_s start,stop; // Измерение времени
  
  TIM_TimeStamp(&start); // Начало измерения времени
  send_res = SendData(fr);
  TIM_TimeStamp(&stop); // Конец измерения времени
  
  uint32_t passed = TIM_passedTime(&start, &stop);
  RI_UPTIME += (float)passed/1000; // Микросекунды в милисекунды
  
  // В случаи ошибки передачи увеличиваем счетчик RI_CCA_REJECT
  if (!send_res)
    RI_CCA_REJECT++;
  
  return send_res;
}

/**
@brief Отправка сообщения
@detail Время отправки с шифрованием 10 байт в мкс. Merge = 155, Crypt = 239,
 Load TX = 30, RSSI_OK = 328, ISTXON = 11, SFD(192us+5*8*4us) = 400, TX = 370, Full = 1535.
Время работы радио RSSI_OK+ISTXON+SFD+TX = 1109 us
@return true в случаи успеха
*/
static bool SendData(frame_s *fr)
{
////TimeStamp_s ts_start, ts_frame_merge, ts_crypt, ts_load_tx, 
////ts_rssistat, ts_istxon, ts_sfd, ts_stop;
  
////TIM_TimeStamp(&ts_start);  
  
  // Обязательно скопируем данные для отправки.
  // Данные могут шифроваться и если их не отправили, то payload измениться.
  uint8_t data_size = fr->len;
  uint8_t *data = re_malloc(data_size);
  ASSERT_HALT(data !=NULL, "No memory");
  re_memcpy(data, fr->payload, data_size);
    
////TIM_TimeStamp(&ts_frame_merge);  
  bool result = true;
  switch(true)
  {
    case true:
      // Шифруем данные при необходимости
      RI_BitRawCrypt(data, data_size);
////TIM_TimeStamp(&ts_crypt); 
      // Копируем данные в буфер. Очистка буфера автоматическая
      LoadTXData(data, data_size);
////TIM_TimeStamp(&ts_load_tx); 
      // Для начала передачи по команде STXONCCA нужно включить приемник
      ISRXON();
      // Ждем пока статус RSSI_VALID станет true
      while(!RSSISTAT);
////TIM_TimeStamp(&ts_rssistat); 
      // Очищаем флаг завершения передачи сообщения
      RFIRQF1 &= ~RFIRQF1_TXDONE;
      RFIRQF0 &= ~RFIRQF0_SFD;
      
      // Время отправки сообщения измеряется в тактах сетевого времени NTMR
      // Он работатаеи намного лучше чем MAC таймер(меньше погрешность)
      // Отправка в обозначенное время или по факту готовности
      uint16_t timer = 0; // Для отлалки. 
      if (fr->meta.SEND_TIME != 0)
        timer = NT_WaitTime(fr->meta.SEND_TIME);

      // Начинаем передачу данных
      // Transmission of preamble begins 192 μs after the STXON or STXONCCA 
      // command strobe
      // 192+(4(pream)+1(sfd))*8bit*4us = 352 мкс. Измерил 360мкс
      // Измерил в тактах NT_GetTimer() получилось 13 тиков. 396 мкс 
      ISTXONCCA();
////TIM_TimeStamp(&ts_istxon); 
      // Произошла ошибка передачи если SAMPLED_CCA false
      if (!(FSMSTAT1 & 1<<3)) //SAMPLED_CCA == 0
      {
        result = false;
        break;
      }

      // Ждем завершения отправки SFD
      while (!(RFIRQF0 & RFIRQF0_SFD));
      fr->meta.TIMESTAMP = NT_GetTime(); 
////TIM_TimeStamp(&ts_sfd); 
      // Проверим переданно ли сообщение TX_FRM_DONE
      while (!(RFIRQF1 & RFIRQF1_TXDONE));
      break;
  }
////TIM_TimeStamp(&ts_stop);

  re_free(data);
  ISRFOFF();
////  LOG(MSG_ON | MSG_INFO | MSG_TRACE, 
////"Merge = %lu. Crypt = %lu. Load TX = %lu. RSSI_OK = %lu. ISTXON = %lu. SFD = %lu. TX = %lu. FULL = %lu \n", 
////      TIM_passedTime(&ts_start, &ts_frame_merge),
////      TIM_passedTime(&ts_frame_merge, &ts_crypt),
////      TIM_passedTime(&ts_crypt, &ts_load_tx),
////      TIM_passedTime(&ts_load_tx, &ts_rssistat),
////      TIM_passedTime(&ts_rssistat, &ts_istxon),
////      TIM_passedTime(&ts_istxon, &ts_sfd),
////      TIM_passedTime(&ts_sfd, &ts_stop),
////      TIM_passedTime(&ts_start, &ts_stop)
////      );
  
  if (result)
    return true;
  return false; 
}

/**
@brief Загрузка данных для передачи в буфер. 
@detail Поля LEN, FCS1, FCS2 добавляются автоматически
@param[in] src указатель на данные
@param[in] len размер данных
*/
static void LoadTXData(uint8_t *src, uint8_t len)
{
  // Очищаем буфер передатчика
  ISFLUSHTX(); 
  // Поле LEN на два байта больше
  RFD = len + 2;
  
  for (uint8_t i = 0 ; i < len; i++)
    RFD = src[i];
  
  // Добавляем CRC1,2
  RFD = 0x00;
  RFD = 0x00;
};

/**
@brief Выгружаем принятый пакет из радио
@param[out] src указатель на буфер приемника
@param[in] len размер выгружаемых данных
*/
static void UnLoadRXData(uint8_t *src, uint8_t len)
{
  for (uint8_t i = 0 ; i < len; i++)
    src[i] = RFD;
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
RI_CRC_ERROR. Дешифрует данные при необходимости. Отмечает время прихода SFD 
 в тактах сетевого времени .
\param[in] timeout Время ожидания данных в милисекундах
\return Возвращает NULL если данных нет
*/
frame_s* RI_Receive(uint16_t timeout)
{
  // Устанавливаем частоту передачи пакета
  RI_cfg();
  uint16_t SFD_TimeStamp;
  
  // Принимаем пакет 
  uint32_t timeout_us = timeout*1000UL; // Переводим мс->мкс
  TimeStamp_s start,stop; // Измерение времени
  TIM_TimeStamp(&start); // Начало измерения времени работы радио
  bool recv_res = RecvData(timeout_us, &SFD_TimeStamp);
  TIM_TimeStamp(&stop); // Конец измерения времени радио
  uint32_t passed = TIM_passedTime(&start, &stop);
  RI_UPTIME += (float)passed/(float)1000; // Микросекунды в милисекунды
  
  // Если ничего не приняли возвращаем NULL
  if (!recv_res)
    return NULL;
  
  uint8_t frame_size = RXFIFOCNT; // Количество принятых данных
  
  // Минимальный размер FCS1,FCS2 - 2 байта. LEN не включен в подсчет 
  if (frame_size <= 2)
    return NULL;
  
  // Выгружаем данные из приемника
  uint8_t *frame_raw = re_malloc(frame_size); 
  UnLoadRXData(frame_raw, frame_size);
  
  int8_t  FCS1 = frame_raw[frame_size - 2]; // RSSI
  uint8_t FCS2 = frame_raw[frame_size - 1]; // bit7 = CRCOK, bit[6..0] Corr_val
  uint8_t LEN_F = frame_raw[0]; // Поле LEN
  
  // Проверим поле LEN. Размер данных в заголовке должен совпадать
  // с фактической длинной принятых данных
  if (LEN_F != frame_size - 1)
  {
    re_free(frame_raw);
    RI_CRC_ERROR ++;
    return NULL;
  }
  
  // Проверим поле CRCOK
  if (!(FCS2 && 1<<7))
  {
    re_free(frame_raw);
    RI_CRC_ERROR ++;
    return NULL;
  }
  
  // Создаем буфер, последнии два байта FCS1,2 и поле LEN не копируем
  frame_s *raw_frame = frame_create();
  frame_addHeader(raw_frame, &frame_raw[1], frame_size - 2);
  // Декодируем поток если нужно
  RI_BitRawDecrypt(raw_frame->payload, raw_frame->len);
  
  // Копируем метку времени SFD
  raw_frame->meta.TIMESTAMP = SFD_TimeStamp;
  
  // Расчитываем мощность принятого сигнала
  raw_frame->meta.RSSI_SIG =  FCS1 + RSSI_OFFSET;
  // Расчитываем качество сигнала
  uint8_t corr = FCS2 & 0x7F;
  raw_frame->meta.LIQ = LIQ_CALC(corr); // Значение коэфф. корреляции

  raw_frame->meta.CH = RADIO_CFG.CH;

  re_free(frame_raw);
  return raw_frame;
}

/**
@brief Прием данных из эфира
@param[in] timeout_us время ожидания в микросекундах
@param[out] SFD_TimeStamp отметка времени завершения приема SFD
*/
static bool RecvData(uint32_t timeout_us, uint16_t *SFD_TimeStamp)
{
  TimeStamp_s start,stop; // Измерение времени  
  ISFLUSHRX(); // Очищаем буфер приема
  
  TIM_TimeStamp(&start);
  
  // Очищаем флаг завершения передачи сообщения и приема SFD
  RFIRQF0 &= ~RFIRQF0_RXPKTDONE;  
  RFIRQF0 &= ~RFIRQF0_SFD; 
  ISRXON(); // Включаем радиопередатчик

  bool time_out = false; // Истекло время ожидания пакета
  bool packet_received = false; // Приняли пакет
  bool sfd_received = false; // Приняли sfd
  
  // Цикл приема пакета
  while (true)
  {
    TIM_TimeStamp(&stop);
    if (TIM_passedTime(&start, &stop) >= timeout_us)
    {
      time_out = true;
      break;
    }
   
    // Принят сигнал SFD
    if ((RFIRQF0 & RFIRQF0_SFD))
    {
      *SFD_TimeStamp = NT_GetTime(); 
      sfd_received = true;
    }
    
    //  Завершен ли прием сообщения RX_FRM_DONE (RXPKTDONE)
    if ((RFIRQF0 & RFIRQF0_RXPKTDONE))
    {
      packet_received = true;
      break;
    }
  } // while 
  
  ISRFOFF();
  if (packet_received && sfd_received && !time_out)
    return true;
  else
    return false;
}

/*!
\brief Возвращает количество ошибок возникших с момента иницилизации
\return Количество CRC ошибок
*/
uint32_t RI_GetCRCError(void)
{
  return RI_CRC_ERROR;
}

/*!
\brief Возвращает количестов отказов передачи пакета из-за занятости канала.
\return Количество отказов CCA
*/
uint32_t RI_GetCCAReject(void)
{
  return RI_CCA_REJECT;
}

/*!
\brief Возвращает суммарное время работы радио в режиме прием/передача
\return Вермя в милисекундах
*/
float RI_GetUptime(void)
{
  return RI_UPTIME;
}

/**
@breif Подссчет количества бит в байте
@return Возвращает количество бит
*/
static uint8_t bits_count(uint8_t value) {
  int ret = 0;
  for ( ; value; value = value >> 1 )
    ret += value & 1;
  return ret;
}

/**
@brief Возращает 1 байт случайного числа
@details Радио выдает всего лишь дви бита, нам нужно 8 
*/
static inline uint8_t getRNDByte(void)
{
  uint8_t val = 0;
  val |= RFRND << 0;
  val |= RFRND << 2;
  val |= RFRND << 4;
  val |= RFRND << 6;
  return val;
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
  while ( bit_cnt <= 2 || bit_cnt >= 6 )
  {
    rnd_val = getRNDByte();
    while (rnd_val == getRNDByte());
    rnd_val = getRNDByte();
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
     
  RI_cfg();
  FREQCTRL = 0x00; // Выбираем не используемую частоту

  // TODO По какой то причине OP_EXE не выполняет команду.
  // регистра RFST читается как 0xD0. это его состояние при reset
  // Включаем демодулятор
  ISRXON();
  
  // Ждем пока статус RSSI_VALID станет true
  while(!RSSISTAT);
  
  // Ждем случайных чисел
  while (RFRND == 0);
  
  // Настраиваем ядро случайного генератора
  rnd_core = readRandom();
  rnd_core |= (unsigned int)readRandom()<<8;
  srand(rnd_core);
  
  // Включаем демодулятор
  ISRFOFF();
  // Первая генерация случайного числа занимает много времени.
  // Влияло на работу радио, так как использовались случайные посылки
  rand(); 
}

/*!
\brief Установка разрешения шифрования потока данных
\param[in] true - включить шифрование
*/
void RI_StreamCrypt(bool state)
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
  
  AES_StreamCoder(true, src, src, KEY, IV, size);
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
   
  AES_StreamCoder(false, src, src, KEY, IV, size);
}
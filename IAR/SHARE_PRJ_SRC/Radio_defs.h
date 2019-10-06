
// Это Immediate Strobe instructions (ISxxx). Запись в командный регистр
// приводит к немедленному исполнению. Команды записываются в RFST.
// Если RFST = 0, то контроллер готов выполнять следующию инструкцию.
enum RADIO_OPCODE {
  OP_SRXON = 0xD3, OP_STXON = 0xD9, OP_STXONCCA = 0xDA, OP_SSAMPLECCA = 0xDB,
  OP_SRFOFF = 0xDF, OP_SFLUSHRX = 0xDD, OP_SFLUSHTX = 0xDE
}
union MDMCTRL0_u
{
  uint8_t value;
  struct {
    uint8_t TX_FILTER:1;
    uint8_t PREAMBLE_LENGTH:4;
    uint8_t DEMOD_AVG_MODE:1;
    uint8_t DEM_NUM_ZEROS:2;
  } bits;
};

union MDMCTRL1_u
{
  uint8_t value;
  struct {
    uint8_t CORR_THR:5;
    uint8_t CORR_THR_SFD:1;
  } bits;
};

union MDMTEST1_u
{
  uint8_t value;
  struct {
    uint8_t RESERVED:1;
    uint8_t MODULATION_MODE:1;
    uint8_t RFC_SNIFF_EN:1;
  } bits;
};

union FRMCTRL0_u
{
  uint8_t value;
  struct {
    uint8_t TX_MODE:2;
    uint8_t RX_MODE:2;
    uint8_t ENERGY_SCAN:1;
    uint8_t AUTOACK:1;
    uint8_t AUTOCRC:1;
    uint8_t APPEND_DATA_MODE:1;
  } bits;
};

union FRMCTRL1_u
{
  uint8_t value;
  struct {
    uint8_t SET_RXENMASK_ON_TX:1;
    uint8_t IGNORE_TX_UNDERF:1;
    uint8_t PENDING_OR:1;
  } bits;
};


union FREQTUNE_u // Можно слегка сместить частоту, чтобы нас не снифирелии
{
  uint8_t value;
  struct {
    uint8_t XOSC32M_TUNE:4;    
  } bits;
};


union FREQCTRL_u
{
  uint8_t value;
  struct {
    uint8_t FREQ:7;    
  } bits;
};


union FSMSTAT0_u
{
  uint8_t value;
  struct {
    uint8_t FSM_FFCTRL_STATE:6;
    uint8_t CAL_RUNNING:1;   
  } bits;
};

union FSMSTAT1_u
{
  uint8_t value;
  struct {
    uint8_t RX_ACTIVE:1;
    uint8_t TX_ACTIVE:1;
    uint8_t LOCK_STATUS:1;
    uint8_t SAMPLED_CCA:1;
    uint8_t CCA:1;
    uint8_t SFD:1;
    uint8_t FIFOP:1;
    uint8_t FIFO:1;
  } bits;
};

union CCACTRL1_u
{
  uint8_t value;
  struct {
    uint8_t CCA_HYST:3;
    uint8_t CCA_MODE:2;     
  } bits;
};

union RSSISTAT_u
{
  uint8_t value;
  struct {
    uint8_t RSSI_VALID:1;     
  } bits;
};


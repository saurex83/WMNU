import connector

DEFAULT_KEY = b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10'
DEFAULT_IV =  b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10'
DEFAULT_PANID = b'\x12'
DEFAULT_RTC = b'\x00\x00\x01\x05'
DEFAULT_SYS_CH = b'\x0B'
SYS_TS = b'\x00'
DEFAULT_TX_POW = b'\xF5'  # xF5 +4.5 x05 -22

def TransiverStart(UART_CONN):
    UART_CONN.cmd03_ON_OFF(False)
    UART_CONN.cmd06_Reset()
    UART_CONN.cmd01_Set_panid(DEFAULT_PANID)
    UART_CONN.cmd02_Set_RTC(DEFAULT_RTC)
    UART_CONN.cmd04_Set_IV(DEFAULT_IV)
    UART_CONN.cmd05_Set_KEY(DEFAULT_KEY)
    UART_CONN.cmd0F_Set_SYS_CH(DEFAULT_SYS_CH)
    UART_CONN.cmd10_Set_TX_power(DEFAULT_TX_POW) 
    UART_CONN.cmd03_ON_OFF(True)
    UART_CONN.cmd07_OpenSlot(SYS_TS, DEFAULT_SYS_CH)

def TransiverStop(UART_CONN):
    UART_CONN.cmd03_ON_OFF(False)
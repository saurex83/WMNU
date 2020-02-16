import time
import connector
import frame


def cmdx00(UART_CONN):
    UART_CONN.cmd03_ON_OFF(False)
    if UART_CONN.cmd00_Status():
        print("ОШИБКА. Сеть должна не раздаваться")

    UART_CONN.cmd03_ON_OFF(True)
    if not UART_CONN.cmd00_Status():
        print("ОШИБКА. Сеть должна раздаваться")

    UART_CONN.cmd03_ON_OFF(False)
    if UART_CONN.cmd00_Status():
        print("ОШИБКА. Сеть должна не раздаваться")

def cmdx01(UART_CONN):
    UART_CONN.cmd03_ON_OFF(True)
    if UART_CONN.cmd01_Set_panid(b'\x01'):
        print("ОШИБКА. panid нельзя установить так как сеть раздается")
    UART_CONN.cmd03_ON_OFF(False)
    if not UART_CONN.cmd01_Set_panid(b'\x01'):
        print("ОШИБКА. panid должен был установиться")  

def cmdx02(UART_CONN):
    UART_CONN.cmd03_ON_OFF(True)
    if UART_CONN.cmd02_Set_RTC(b'\x01\x01\x01\x01'):
        print("ОШИБКА. rtc нельзя установить так как сеть раздается")
    UART_CONN.cmd03_ON_OFF(False)
    if not UART_CONN.cmd02_Set_RTC(b'\x01\x01\x01\x01'):
        print("ОШИБКА. rtc должен был установиться")               

def cmdx04(UART_CONN):
    UART_CONN.cmd03_ON_OFF(True)
    if UART_CONN.cmd04_Set_IV(b'\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01'):
        print("ОШИБКА. IV нельзя установить так как сеть раздается")
    UART_CONN.cmd03_ON_OFF(False)
    if not UART_CONN.cmd04_Set_IV(b'\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01'):
        print("ОШИБКА. IV должен был установиться")  

def cmdx05(UART_CONN):
    UART_CONN.cmd03_ON_OFF(True)
    if UART_CONN.cmd05_Set_KEY(b'\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01'):
        print("ОШИБКА. KEY нельзя установить так как сеть раздается")
    UART_CONN.cmd03_ON_OFF(False)
    if not UART_CONN.cmd05_Set_KEY(b'\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01'):
        print("ОШИБКА. KEY должен был установиться")  


def cmdx07(UART_CONN):
    UART_CONN.cmd03_ON_OFF(True)
    if not UART_CONN.cmd07_OpenSlot(b'\x10', b'\x10'):
        print("ОШИБКА. открыть можно сеть раздается")
    UART_CONN.cmd03_ON_OFF(False)
    if UART_CONN.cmd07_OpenSlot(b'\x10', b'\x10'):
        print("ОШИБКА. открыть нельзя сеть нераздается") 
        
def cmdx08(UART_CONN):
    UART_CONN.cmd03_ON_OFF(True)
    if not UART_CONN.cmd08_CloseSlot(b'\x10'):
        print("ОШИБКА. закрыть можно сеть раздается")
    UART_CONN.cmd03_ON_OFF(False)
    if UART_CONN.cmd08_CloseSlot(b'\x10'):
        print("ОШИБКА. закрыть нельзя сеть нераздается") 


def cmdx09(UART_CONN):
    UART_CONN.cmd03_ON_OFF(True)
    if UART_CONN.cmd09_RX_buff_size() !=0:
        print("Буфер RX должен быть пуст")
     
def cmdx0A(UART_CONN):
    UART_CONN.cmd03_ON_OFF(True)
    if UART_CONN.cmd0A_TX_buff_size() !=0:
        print("Буфер TX должен быть пуст")     

def cmdx0B(UART_CONN):
    fr = frame.Frame()
    fr.META['TS'] = 2
    fr.META['CH'] = 15
    fr.META['TX_METHOD'] = frame.TX_BROADCAST
    fr.META['ACK'] = 3
    fr.META['PID'] = 4
    fr.META['IPP'] = 5
    fr.META['ETX'] = 6
    fr.META['FDST'] = 55
    fr.META['FSRC'] = 77
    fr.META['NDST'] = 12
    fr.META['NSRC'] = 15
    fr.META['TIMESTAMP'] = 0
    fr.PAYLOAD = b'abcdef'
    data = fr.assemle()
    UART_CONN.cmd0B_tx_frame(data)
    if UART_CONN.cmd0A_TX_buff_size() !=1:
        print("Буфер TX должен содержать 1 пакет")      

def CmdChecker(UART_CONN):
    cmdtocheck = [cmdx00, cmdx01,cmdx02,cmdx04,cmdx05, cmdx07, cmdx08,cmdx09,cmdx0A,cmdx0B ]

    for cmd in cmdtocheck:
        cmd(UART_CONN)

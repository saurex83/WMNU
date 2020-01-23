import time
import connector
import matplotlib.pyplot as plt
import numpy as np

DEFAULT_KEY = b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F'
DEFAULT_IV =  b'\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F'


def detect_wifi_AP(UART_CONN):
    UART_CONN.cmd03_ON_OFF(False)
    WIFI_CH = [18,43,68] # 18 - 1k 43-6k, 68 - 11k
    timeout = (5000).to_bytes(2, byteorder='big')
    RES = []
    
    for ch in WIFI_CH:
        ch_bytes = ch.to_bytes(1, byteorder='big') 
        rssi = UART_CONN.cmd0D_Energy_Scan(ch_bytes, timeout)
        if (rssi > -60):
            print("Точка доступа на %d канале"%(ch))


def energy__scan(UART_CONN):

    UART_CONN.cmd03_ON_OFF(False)
    timeout = (5000).to_bytes(2, byteorder='big') 
    ZIGBEE_CH = [11,16,21,26,31,36,41,46,51,56,61,66,71,76,81,86,91,96,101,106,111]
    MAX_CH = 113
    
    ax_x = []
    ax_y = []
    for ch in ZIGBEE_CH: #range(MAX_CH):
        ax_x = ax_x + [2394+ch]
        ch_bytes = ch.to_bytes(1, byteorder='big') 
        rssi = UART_CONN.cmd0D_Energy_Scan(ch_bytes, timeout)
        ax_y = ax_y + [abs(rssi)]
        print('Канал %d, энергия: %d дБм'%(ch, rssi))

    fig, ax = plt.subplots(figsize=(5, 3))
    ax.stackplot(np.array(ax_x), np.array(ax_y))
    ax.set_title('Combined debt growth over time')
    ax.legend(loc='upper left')
    ax.set_ylabel('Total debt')
    ax.set_xlim(xmin=2394, xmax=2394+MAX_CH)
    fig.tight_layout()
    plt.show()

def main():
    UART_CONN = connector.Connector()

#    detect_wifi_AP(UART_CONN)
 #   exit(0)

    energy__scan(UART_CONN)
    exit(0)

    while(True):
        UART_CONN.cmd06_Reset()
        UART_CONN.cmd00_Status()
        UART_CONN.cmd01_Set_panid(b'\x71\xf4')
        UART_CONN.cmd02_Set_RTC(b'\x00\x00\x01\x05')
        UART_CONN.cmd03_ON_OFF(False)
        UART_CONN.cmd0E_Set_SYNC_CH(b'\x0B')
        UART_CONN.cmd01_Set_panid(b'\x71\xf4')
        UART_CONN.cmd02_Set_RTC(b'\x00\x00\x01\x05')
        UART_CONN.cmd04_Set_IV(DEFAULT_IV)
        UART_CONN.cmd05_Set_KEY(DEFAULT_KEY)
        UART_CONN.cmd07_OpenSlot(b'\x01',b'\x0B')
        UART_CONN.cmd08_CloseSlot(b'\x01')
        UART_CONN.cmd03_ON_OFF(True)
        UART_CONN.cmd07_OpenSlot(b'\x01',b'\x0B')
        UART_CONN.cmd08_CloseSlot(b'\x01')
        UART_CONN.cmd09_RX_buff_size()
        UART_CONN.cmd0A_TX_buff_size()
        UART_CONN.cmd0E_Set_SYNC_CH(b'\x0B')
        
        UART_CONN.cmd03_ON_OFF(False)
            

if __name__ == "__main__":
    main()
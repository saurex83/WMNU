import time
import connector
import frame
import ip
import udp
import eth
import matplotlib.pyplot as plt
import numpy as np
import transiver_control
import neigh_test

DEFAULT_KEY = b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10'
DEFAULT_IV =  b'\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10'

def SEND_ETHERNET(UART_CONN):
    fr = frame.Frame()
    eth_h = eth.ETH()
    ip_h = ip.IP()
    udp_h = udp.UDP()

    udp_h.META['PORT'] = 10
    udp_h.PAYLOAD = b'HELLO!'

    eth_h.META['PID'] = 1 # 1 - IP
    eth_h.META['NETID'] = transiver_control.DEFAULT_PANID
    eth_h.META['NDST'] = 0xffff # широковещание
    eth_h.META['NSRC'] = 0x0000 # от шлюза
    
    ip_h.META['ETX'] = 1 
    ip_h.META['FDST'] = 0xffff
    ip_h.META['FSRC'] = 0x1234 
    ip_h.META['IPP'] = 0

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
    fr.PAYLOAD = eth_h.assemle() + ip_h.assemle() + udp_h.assemle()
    data = fr.assemle()

    UART_CONN.cmd03_ON_OFF(False)
   # UART_CONN.cmd06_Reset()
    UART_CONN.cmd01_Set_panid(b'\x12')
   # UART_CONN.cmd02_Set_RTC(b'\x00\x00\x01\x05')
    UART_CONN.cmd04_Set_IV(DEFAULT_IV)
    UART_CONN.cmd05_Set_KEY(DEFAULT_KEY)
    UART_CONN.cmd0F_Set_SYS_CH(b'\x0B')
    UART_CONN.cmd10_Set_TX_power(b'\xF5')  # xF5 +4.5 x05 -22 
    UART_CONN.cmd03_ON_OFF(True)
    UART_CONN.cmd07_OpenSlot(b'\x22',b'\x14')

    while(True):
        if UART_CONN.cmd0A_TX_buff_size() == 0:
            UART_CONN.cmd0B_tx_frame(data)
     #   UART_CONN.cmd09_RX_buff_size()
        if UART_CONN.cmd09_RX_buff_size() != 0:
            rx_frame = UART_CONN.cmd0C_get_rx_frame()
            fr_parse = frame.Frame()
            fr_parse.parse(rx_frame)
            print('TIMESTAMP :%d, RSSI :%d'%(fr_parse.META['TIMESTAMP'],fr_parse.META['RSSI_SIG'] ))
            print(rx_frame)
 
        time.sleep(1)  

def SEND_FRAME(UART_CONN):
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

    UART_CONN.cmd03_ON_OFF(False)
   # UART_CONN.cmd06_Reset()
   # UART_CONN.cmd01_Set_panid(b'\x71\xf4')
   # UART_CONN.cmd02_Set_RTC(b'\x00\x00\x01\x05')
    UART_CONN.cmd04_Set_IV(DEFAULT_IV)
    UART_CONN.cmd05_Set_KEY(DEFAULT_KEY)
    UART_CONN.cmd03_ON_OFF(True)
    UART_CONN.cmd07_OpenSlot(b'\x22',b'\x14')

    while(True):
        if UART_CONN.cmd0A_TX_buff_size() == 0:
            UART_CONN.cmd0B_tx_frame(data)
     #   UART_CONN.cmd09_RX_buff_size()
        if UART_CONN.cmd09_RX_buff_size() != 0:
            rx_frame = UART_CONN.cmd0C_get_rx_frame()
            fr_parse = frame.Frame()
            fr_parse.parse(rx_frame)
            print('TIMESTAMP :%d, RSSI :%d'%(fr_parse.META['TIMESTAMP'],fr_parse.META['RSSI_SIG'] ))
            print(rx_frame)
 
        time.sleep(1)


def energy__scan(UART_CONN):

    UART_CONN.cmd03_ON_OFF(False)
    timeout = (500).to_bytes(2, byteorder='big') 
    ZIGBEE_CH = [11,16,21,26,31,36,41,46,51,56,61,66,71,76,81,86,91,96,101,106,111]
    MAX_CH = 113
    
    ax_x = []
    ax_y = []
    for ch in range(MAX_CH):
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

    transiver_control.TransiverStart(UART_CONN)
    while(True):
        neigh_test.test1(UART_CONN)
    exit(0)
    SEND_ETHERNET(UART_CONN)
    exit(0)
  #  SEND_FRAME(UART_CONN)
  #  exit(0)
  #  energy__scan(UART_CONN)
 #   exit(0)

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
        UART_CONN.cmd0B_tx_frame(b'45df')
        UART_CONN.cmd0C_get_rx_frame()
        UART_CONN.cmd03_ON_OFF(False)
            

if __name__ == "__main__":
    main()
import time
import connector
import frame
import eth
import transiver_control
import neighnbor
import random

def test1(UART_CONN):
    # Посылаем запрос протоколу соседей об отправки карты
    fr = frame.Frame()
    eth_h = eth.ETH()

    eth_h.META['PID'] = 2 # 1 - IP 2 - PID_NP
    eth_h.META['NETID'] = 0x12
    eth_h.META['NDST'] = 0xffff
    eth_h.META['NSRC'] = 0x0000

    fr.META['TS'] = 0 # 0 - системный слот
    fr.META['CH'] = 11 # системная частота
    fr.META['TX_METHOD'] = frame.TX_BROADCAST

    fr.PAYLOAD = eth_h.assemle() + neighnbor.NEIGNBOR().assemble_req()
    data = fr.assemle()
    
    while(True):
        if UART_CONN.cmd0A_TX_buff_size() < 5:
            UART_CONN.cmd0B_tx_frame(data)
            break
        time.sleep(1)
    time.sleep(0.2)

def test2(UART_CONN):
    # Посылаем карты соседей
    fr = frame.Frame()
    eth_h = eth.ETH()
    nb_h = neighnbor.NEIGNBOR()

    eth_h.META['PID'] = 2 # 1 - IP 2 - PID_NP
    eth_h.META['NETID'] = 0x12
    eth_h.META['NDST'] = 0xffff
    eth_h.META['NSRC'] = 0x0000

    fr.META['TS'] = 0 # 0 - системный слот
    fr.META['CH'] = 11 # системная частота
    fr.META['TX_METHOD'] = frame.TX_BROADCAST

    nb_h.META['ch0'] = 14
    nb_h.META['ch1'] = 15
    nb_h.META['ch2'] = 16
    nb_h.META['ts0'] = 5
    nb_h.META['ts1'] = 10
    nb_h.META['ts2'] = 15

    fr.PAYLOAD = eth_h.assemle() + neighnbor.NEIGNBOR().assemble_card()
    data = fr.assemle()
    
    while(True):
        eth_h.META['NSRC'] = random.randrange(1000,3000,1)
        fr.PAYLOAD = eth_h.assemle() + neighnbor.NEIGNBOR().assemble_card()
        data = fr.assemle()
        
        if UART_CONN.cmd0A_TX_buff_size() < 2:
            UART_CONN.cmd0B_tx_frame(data)
            break
        time.sleep(1)
    time.sleep(0.2)
            

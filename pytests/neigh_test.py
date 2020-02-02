import time
import connector
import frame
import eth
import transiver_control
import neighnbor

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
            

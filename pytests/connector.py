import serial
import time

COM_PORT = '/dev/ttyUSB0'
BOUD = 115200   #2000000
RX_TIMEOUT = 50e-3
MAX_RX_BYTES = 255

#Типы ответов
ATYPE_PAR_ERR = 0
ATYPE_CMD_ERR = 1
ATYPE_CMD_OK = 2

# Ошибки парсера
PAR_CRC16 = 1
PAR_NOCMD = 2

# Ошибки команды
CMD_LEN = 1
CMD_ARG_VAL = 2
CMD_SEEDING = 3
CMD_NOSEEDING = 4
CMD_TX_FULL = 5
CMD_RX_EMPTY = 6 

# Расшифровка ошибок
BIND_PAR_ERR = {
  PAR_CRC16 : 'Ошибка CRC16',
  PAR_NOCMD : 'Команды не существует'  
}

BIND_CMD_ERR = {
    CMD_LEN : 'Неверная длина аргументов',
    CMD_ARG_VAL: 'Неверное значение аргумета',
    CMD_SEEDING: 'Раздача сети должна быть отключена',
    CMD_NOSEEDING: 'Раздача сети должна быть включена',
    CMD_TX_FULL : 'Очередь на передачу полна',
    CMD_RX_EMPTY : 'Буфер приемника пуст'
}
class Connector:
    def __init__(self):
        self.ser = serial.Serial(COM_PORT, BOUD, timeout = RX_TIMEOUT)
        self.ser.flush()
    
    def _calcCRC16(self, data):
        # Расчет CRC16. Возращает 2 байта
        return b'\xAA\xAA' # Пока что заглушка

    def _write(self,data):
        # Добавляем байт длинны и CRC16 к посылке
        len_bytes = (len(data) + 2).to_bytes(1, byteorder='big') 
        send_data = len_bytes + data + self._calcCRC16(data)    
        self.ser.write(send_data)

    def _read(self):
        RET = {'err' : '', 'data' : b''}

        # Читаем данные из порта
        data = self.ser.read(MAX_RX_BYTES)
        
        if (len(data) == 0):
            RET['err'] = 'Нет данных'
            return RET

        # Размер указанный в первом байте не соответсвует фактическому
        if (data[0] != len(data) - 1):
            RET['err'] = 'Неверная длина'
            return RET

        # Ответ от парсера об ошибке
        if (data[1] == ATYPE_PAR_ERR): 
            RET['err'] = BIND_PAR_ERR[data[2]] 
            return RET

        # Сообщение команды об ошибке
        if (data[1] == ATYPE_CMD_ERR): 
            RET['err'] = BIND_CMD_ERR[data[2]]
            return RET

        # Команда выполнена
        if (data[1] == ATYPE_CMD_OK): 
            # Возвращаем данные без LEN, TYP и CRC16
            RET['data'] = data[2:-2]
            return RET

    def cmd00_Status(self):
        self._write(b'\x00')
        answ = self._read()

        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b'\x00'):
            print('Трансивер не раздает сеть')
            return False

        if (answ['data'] == b'\x01'):
            print('Трансивер раздает сеть')
            return True

    def cmd01_Set_panid(self, panid):
        #panid тип bytes длинной 2 байта
        cmd = b'\x01' + panid
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b''):
            print('Panid изменен!')
            return True
        
    def cmd02_Set_RTC(self, rtc):
        #rtc тип bytes длинной 4 байта
        cmd = b'\x02' + rtc
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b''):
            print('RTC изменен!')
            return True

    def cmd03_ON_OFF(self, on_off):
        #rtc тип bool
        if (on_off):
            cmd = b'\x03' + b'\x01'
        else:
            cmd = b'\x03' + b'\x00'

        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b''):
            if (on_off):
                print('Раздача сети включена!')
            else:
                print('Раздача сети остановлена!')
            return True
        
    def cmd04_Set_IV(self, IV):
        #IV тип bytes размер 16
        cmd = b'\x04' + IV
        
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b''):
            print('Новый вектор IV загружен!')
            return True

    def cmd05_Set_KEY(self, KEY):
        #IV тип bytes размер 16
        cmd = b'\x05' + KEY
        
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b''):
            print('Новый KEY загружен!')
            return True

    def cmd06_Reset(self):
        #IV тип bytes размер 16
        cmd = b'\x06' + b'\x00' # занчение dont use не на что не влияет
        
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b''):
            print('Трансивер перезагружается!')
            return True

    def cmd07_OpenSlot(self, ts, ch):
        #ts и ch тип bytes размер по 1 байту
        cmd = b'\x07' + ts + ch
        
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b''):
            print('Открыт прием в TS = %d, канал = %d!'%(ts[0],ch[0]))
            return True

    def cmd08_CloseSlot(self, ts):
        #ts тип bytes размер по 1 байту
        cmd = b'\x08' + ts  
        
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b''):
            print('Закрыт прием в TS = %d!'%(ts[0]))
            return True

    def cmd09_RX_buff_size(self):
        cmd = b'\x09'  
        
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        nbrItem = answ['data'][0]
        print('Принято пакетов в RX буфер = %d!'%(nbrItem))
        return nbrItem

    def cmd0A_TX_buff_size(self):
        cmd = b'\x0A'  
        
        self._write(cmd)
        answ = self._read()

        if answ['err'] != '':
            print(answ['err'])
            return False

        nbrItem = answ['data'][0]
        print('Пакетов на передачу TX буфер = %d!'%(nbrItem))
        return nbrItem
        
    def cmd0B_tx_frame(self, frame):
        # frame - бинарный пакет
        cmd = b'\x0B' + frame
        
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False
        return True

    def cmd0C_get_rx_frame(self):
        cmd = b'\x0C'
        
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        print('Принят пакет из буфера RX длинной %d'%(len(answ['data'])))
        return (answ['data'])

    def cmd0E_Set_SYNC_CH(self, ch):
        # ch byte
        cmd = b'\x0E' + ch  
        
        self._write(cmd)
        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False

        if (answ['data'] == b''):
            print('Установлен канал синхросигнала = %d!'%(ch[0]))
            return True

    def cmd0D_Energy_Scan(self, ch, timeout):
        # ch byte from 0 to 113 
        # timeout 2 byte
        cmd = b'\x0D' + ch  + timeout
        
        self._write(cmd)
        time.sleep(int.from_bytes(timeout, byteorder='big')/1000)

        answ = self._read()
        
        if answ['err'] != '':
            print(answ['err'])
            return False
        
        tmp = answ['data']
        rssi = int.from_bytes(tmp, byteorder='big', signed=True)

        #print('Энергия в канале: %d'%(rssi))
        return rssi
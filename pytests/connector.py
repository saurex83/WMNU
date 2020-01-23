import serial
import time

COM_PORT = '/dev/ttyUSB0'
BOUD = 115200   #2000000
RX_TIMEOUT = 50e-3
MAX_RX_BYTES = 255

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
        # Читаем данные из порта
        data = self.ser.read(MAX_RX_BYTES)
        
        if (len(data) == 0):
            return b''

        # Размер не верен
        if (data[0] != len(data) - 1):
            return b''

        if (data[1] == 0x00): # Ответ от парсера об ошибке
            print('Ошибка от парсера TYP: %d'%(data[2]))
            return b''

        if (data[1] == 0x01): # Ответ от обработчика команд
            # Возвращаем данные без LEN, TYP и CRC16
            return data[2:-2]

    def cmd00_Status(self):
        self._write(b'\x00')
        answ = self._read()
        if (len(answ) == 0):
            print('Нет ответа')
            return False

        if (answ == b'\x00'):
            print('Трансивер не раздает сеть')
            return False

        if (answ == b'\x01'):
            print('Трансивер раздает сеть')
            return True

    def cmd01_Set_panid(self, panid):
        #panid тип bytes длинной 2 байта
        cmd = b'\x01' + panid
        self._write(cmd)
        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return False

        if (answ == b'\x01'):
            print('Нельзя поменять panid, так как раздается сеть')
            return False

        if (answ == b'\x00'):
            print('Panid изменен!')
            return True
        
    def cmd02_Set_RTC(self, rtc):
        #rtc тип bytes длинной 4 байта
        cmd = b'\x02' + rtc
        self._write(cmd)
        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return False

        if (answ == b'\x01'):
            print('Нельзя поменять RTC, так как раздается сеть')
            return False

        if (answ == b'\x00'):
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
        
        if (len(answ) == 0):
            print('Нет ответа')
            return False

        if (answ == b'\x00'):
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
        
        if (len(answ) == 0):
            print('Нет ответа')
            return False

        if (answ == b'\x00'):
            print('Новый вектор IV загружен!')
            return True

        if (answ == b'\x01'):
            print('Нельзя загрузить IV, так как раздается сеть')
            return True

    def cmd05_Set_KEY(self, KEY):
        #IV тип bytes размер 16
        cmd = b'\x05' + KEY
        
        self._write(cmd)
        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return False

        if (answ == b'\x00'):
            print('Новый KEY загружен!')
            return True

        if (answ == b'\x01'):
            print('Нельзя загрузить KEY, так как раздается сеть')
            return True

    def cmd06_Reset(self):
        #IV тип bytes размер 16
        cmd = b'\x06' + b'\x00' # занчение dont use не на что не влияет
        
        self._write(cmd)
        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return False

        if (answ == b'\x00'):
            print('Трансивер перезагружается!')
            return True

    def cmd07_OpenSlot(self, ts, ch):
        #ts и ch тип bytes размер по 1 байту
        cmd = b'\x07' + ts + ch
        
        self._write(cmd)
        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return False

        if (answ == b'\x00'):
            print('Открыт прием в TS = %d, канал = %d!'%(ts[0],ch[0]))
            return True
        
        if (answ == b'\x01'):
            print('Сеть долна раздаваться для открытия приема')
            return False

    def cmd08_CloseSlot(self, ts):
        #ts тип bytes размер по 1 байту
        cmd = b'\x08' + ts  
        
        self._write(cmd)
        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return False

        if (answ == b'\x00'):
            print('Закрыт прием в TS = %d!'%(ts[0]))
            return True
        
        if (answ == b'\x01'):
            print('Сеть долна раздаваться для закрытия приема')
            return False

    def cmd09_RX_buff_size(self):
        cmd = b'\x09'  
        
        self._write(cmd)
        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return 0

        print('Принято пакетов в RX буфер = %d!'%(answ[0]))
        return answ[0]

    def cmd0A_TX_buff_size(self):
        cmd = b'\x0A'  
        
        self._write(cmd)
        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return 0

        print('Пакетов на передачу TX буфер = %d!'%(answ[0]))
        return answ[0]
        
    def cmd0E_Set_SYNC_CH(self, ch):
        # ch byte
        cmd = b'\x0E' + ch  
        
        self._write(cmd)
        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return 0

        if (answ == b'\x00'):
            print('Установлен канал синхросигнала = %d!'%(ch[0]))
            return True
        
        if (answ == b'\x01'):
            print('Сеть должна быть отключена для установки синхроканала')
            return False

    def cmd0D_Energy_Scan(self, ch, timeout):
        # ch byte from 0 to 113 
        # timeout 2 byte
        cmd = b'\x0D' + ch  + timeout
        
        self._write(cmd)
        time.sleep(int.from_bytes(timeout, byteorder='big')/1000)

        answ = self._read()
        
        if (len(answ) == 0):
            print('Нет ответа')
            return 0
        
        tmp = answ[1:2]
        rssi = int.from_bytes(tmp, byteorder='big', signed=True)

        #print('Энергия в канале: %d'%(rssi))
        return rssi
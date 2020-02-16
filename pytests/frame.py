import struct

# Структура для разбора байт + байт длинны в конце
# H - uint16_t 
# b - int8_t 
# B - uint8_t 


META_S = struct.Struct('< H b b B B B H H B H H B B H B')

TX_BROADCAST = 0
TX_UNICAST = 1

class Frame():
    META = {
        'TIMESTAMP': 0,
        'RSSI_SIG':0,
        'LIQ':0,
        'TS':0,
        'CH':0,
        'PID':0,
        'NDST' : 0,
        'NSRC' : 0,
        'ETX' : 0,
        'FDST' : 0,
        'FSRC' : 0,
        'IPP' : 0,
        'TX_METHOD' : 0,
        'SEND_TIME' : 0,
        }
    LEN = 0
    PAYLOAD = b''

    def __init__(self):
        pass

    def parse(self, data):
        # Метод разбирает принятые байты и заполняет структуры
        # data байты
        # Первые 15 байт это мета+байт len
        meta = META_S.unpack(data[0:21])
        self.META['TIMESTAMP'] = meta[0]
        self.META['RSSI_SIG'] = meta[1]
        self.META['LIQ'] = meta[2]
        self.META['TS'] = meta[3]
        self.META['CH'] = meta[4]
        self.META['PID'] = meta[5]
        self.META['NDST'] = meta[6]
        self.META['NSRC'] = meta[7]
        self.META['ETX'] = meta[8]
        self.META['FDST'] = meta[9]
        self.META['FSRC'] = meta[10]
        self.META['IPP'] = meta[11]
        self.META['TX_METHOD'] = meta[12]
        self.META['SEND_TIME'] = meta[13]
        self.LEN = meta[14]

        # После байта длинны идет полезная нагрузка
        self.PAYLOAD = data[21:]

    def assemle(self):
        # Собирает массив байт из метадаты
        values = (self.META['TIMESTAMP'],
                self.META['RSSI_SIG'],
                self.META['LIQ'],
                self.META['TS'], 
                self.META['CH'], 
                self.META['PID'], 
                self.META['NDST'], 
                self.META['NSRC'], 
                self.META['ETX'], 
                self.META['FDST'], 
                self.META['FSRC'],
                self.META['IPP'], 
                self.META['TX_METHOD'], 
                self.META['SEND_TIME'], 
                len(self.PAYLOAD)
             )
        packed = META_S.pack(*values)
        # добавляем полезную нагрузку
        packed = packed + self.PAYLOAD
        return packed
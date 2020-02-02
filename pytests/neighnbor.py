import struct
import ip
import eth


CMD_REQ = b'\x00' 
CARD_S = struct.Struct('< B B B B B B B')

class NEIGNBOR():
    META = {'ts0':0, 'ts1':0, 'ts2':0, 'ch0':0, 'ch1':0, 'ch2':0, 'etx':0}

    def __init__(self):
        pass

    def assemble_req(self):
        return CMD_REQ

    def assemble_card(self):
        values = ( 
                self.META['ts0'],
                self.META['ts1'],
                self.META['ts2'],
                self.META['ch0'],
                self.META['ch1'],
                self.META['ch2'],
                self.META['etx'],
             )
        packed = CARD_S.pack(*values)
        return packed

    def parse_card(self, data):
        # Метод разбирает принятые байты и заполняет структуры
        # data байты
        # Первые 16 байт это мета+байт len
        meta = CARD_S.unpack(data[0:7])
        self.META['ts0'] = meta[0]
        self.META['ts1'] = meta[1]
        self.META['ts2'] = meta[2]
        self.META['ch0'] = meta[3]
        self.META['ch1'] = meta[4]
        self.META['ch2'] = meta[5]
        self.META['etx'] = meta[6]        

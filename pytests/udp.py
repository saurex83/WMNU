import struct

META_S = struct.Struct('< B')
ETH_VER = 1

class UDP():
    META = {
        'PORT' : 0, 
    }

    PAYLOAD = b''

    def __init__(self):
        pass

    def parse(self, data):
        pass

    def assemle(self):
        # Собирает массив байт из метадаты
        values = ( 
                self.META['PORT']
             )
        packed = META_S.pack(values) + self.PAYLOAD
        return packed
import struct

META_S = struct.Struct('< B H H B')
ETH_VER = 1

class IP():
    META = {
        'ETX' : 0, 
        'FDST' : 0, 
        'FSRC' : 0, 
        'IPP' : 0  
    }

    def __init__(self):
        pass

    def parse(self, data):
        pass

    def assemle(self):
        # Собирает массив байт из метадаты
        values = ( 
                self.META['ETX'],
                self.META['FDST'], 
                self.META['FSRC'],
                self.META['IPP']
             )
        packed = META_S.pack(*values)
        return packed
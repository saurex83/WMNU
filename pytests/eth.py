import struct

META_S = struct.Struct('< B B H H')
ETH_VER = 1

class ETH():
    META = {
        'PID' : 0, 
        'NETID' : 0, 
        'NDST' : 0, 
        'NSRC' : 0  
    }

    def __init__(self):
        pass

    def parse(self, data):
        pass

    def assemle(self):
        # Собирает массив байт из метадаты
        self.META['PID'] = self.META['PID'] &  0b1111
        ETH_T = self.META['PID'] | (ETH_VER << 5)
        values = ( 
                ETH_T, 
                self.META['NETID'],
                self.META['NDST'], 
                self.META['NSRC']
             )
        packed = META_S.pack(*values)
        return packed
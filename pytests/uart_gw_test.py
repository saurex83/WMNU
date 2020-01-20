import serial
import time


def main():

    COM_PORT = '/dev/ttyUSB0'
    BOUD = 115200#2000000
    WR_SYMB = 254*b'x'
    INFO_CNT = 10
    WRITED = 0

    
    print("Запуск теста ECHO")

    err = 0
    info = 0
    while True:
        with serial.Serial(COM_PORT, BOUD, timeout=2) as ser:
            
            ser.flush()
            # Добавляем размер
            cmd = len(WR_SYMB)
            cmd = cmd.to_bytes(1, byteorder='big') 
            cmd = cmd + WR_SYMB
            #ser.write(b'123')
            #time.sleep(5)
            ser.write(cmd)
            char = ser.read(len(WR_SYMB))
            WRITED = WRITED + 1
            info = info + 1
            
            if (char != WR_SYMB):
                err = err + 1
                print('Принятый размер %d'%(len(char)))
                print('Удачно записанно %d'%(WRITED))
                print(cmd)
                print(char)
                exit()

            if (info > INFO_CNT):
                print("Записанно: %d, Ошибок: %d"%(WRITED, err))
                info = 0


if __name__ == "__main__":
    main()
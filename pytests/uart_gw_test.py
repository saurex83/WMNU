import serial



def main():

    COM_PORT = '/dev/ttyUSB0'
    BOUD = 2000000
    WR_SYMB = b'123456789-'
    INFO_CNT = 10
    WRITED = 0

    
    print("Запуск теста ECHO")

    err = 0
    info = 0
    while True:
        with serial.Serial(COM_PORT, BOUD, timeout=2) as ser:
            
            ser.flush()
            ser.write(WR_SYMB)
            char = ser.read(10)
            WRITED = WRITED + 1
            info = info + 1
            
            if (char != WR_SYMB):
                err = err + 1
                print('Принятый размер %d'%(len(char)))
                print('Удачно записанно %d'%(WRITED))
                print(WR_SYMB)
                print(char)
                exit()

            if (info > INFO_CNT):
                print("Записанно: %d, Ошибок: %d"%(WRITED, err))
                info = 0


if __name__ == "__main__":
    main()
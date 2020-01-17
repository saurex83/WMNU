import serial



def main():

    COM_PORT = '/dev/ttyUSB0'
    BOUD = 2000000
    WR_SYMB = b'123456789012345678901234567890'
    INFO_CNT = 10
    WRITED = 0

    print("Запуск теста ECHO")

    err = 0
    info = 0
    while True:
        with serial.Serial(COM_PORT, BOUD, timeout=1) as ser:
            
            ser.flush()
            ser.write(WR_SYMB)
            char = ser.read(31)
            print(char)
            WRITED = WRITED + 1
            info = info + 1
            
            if (char != WR_SYMB):
                err = err + 1
                print(WRITED)
                print(char)
                while (True):
                    pass

            if (info > INFO_CNT):
                print("Записанно: %d, Ошибок: %d"%(WRITED, err))
                info = 0


if __name__ == "__main__":
    main()
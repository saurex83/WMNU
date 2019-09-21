

# Опции сборки
DEBUG = no
ASSERT = yes
LOG = yes
#USER_APP = tempNode
USER_APP = tests

# Определения
#-------------------------------------------------------------------------------
DEFINES += USE_STDPERIPH_DRIVER
DEFINES += STM32L1XX_MD
DEFINES += STM32L151xB
ifeq ($(ASSERT), yes)
DEFINES += ASSERT_ENABLE
endif
ifeq ($(DEBUG), yes)
DEFINES += DEBUG_ENABLE
endif
ifeq ($(LOG), yes)
DEFINES += LOG_ENABLE
endif

# Имя проекта
#-------------------------------------------------------------------------------
TARGET  = $(USER_APP)

# Включаем makefile пользователя
APP_DIR := apps/$(USER_APP)
include $(APP_DIR)/makefile

# Инструменты
#-------------------------------------------------------------------------------
AS = arm-none-eabi-as
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
CP = arm-none-eabi-objcopy
SZ = arm-none-eabi-size
RM = rm

# Пути к CMSIS, StdPeriph Lib
#-------------------------------------------------------------------------------
CMSIS_PATH         = platform/cmsis
STDPERIPH_PATH = platform/stdperiph
NEWLIB_STUB = platform/newlib
# startup файл
#-------------------------------------------------------------------------------
STARTUP = platform/startup/startup_stm32l1xx_md.s

# Пути поиска исходных файлов
#-------------------------------------------------------------------------------
SOURCEDIRS += nwstack/src
SOURCEDIRS += nwdebuger
SOURCEDIRS += $(STDPERIPH_PATH)
SOURCEDIRS += $(CMSIS_PATH)
SOURCEDIRS += $(NEWLIB_STUB)

# 
EXE_DIR = bin
OBJ_DIR = obj

# Пути поиска хидеров
#-------------------------------------------------------------------------------
INCLUDES += nwstack/inc
INCLUDES += nwdebuger 
INCLUDES += $(CMSIS_PATH)
INCLUDES += $(STDPERIPH_PATH)
INCLUDES += $(NEWLIB_STUB)

# Включаем исходники в зависимости от того что хочет собрать пользователь


# Библиотеки
#-------------------------------------------------------------------------------
LIBPATH +=
LIBS    +=

# Настройки оптимизации компилятора
#-------------------------------------------------------------------------------
ifeq ($(DEBUG), yes)
DEFAULT_OPTIMIZATION += -O0
else
DEFAULT_OPTIMIZATION += -O2
endif


# Настройки компилятора
#-------------------------------------------------------------------------------
# архитектура и система комманд
CFLAGS +=-mthumb -mcpu=cortex-m3
# стандарт языка С
CFLAGS +=-std=gnu99
# Выводить все предупреждения
CFLAGS +=-Wall -pedantic
# Выводить все предупреждения
CFLAGS +=-MMD
# Генерировать отладочную информацию для gdb
CFLAGS +=-ggdb
CFLAGS +=-fno-builtin
CFLAGS +=-Wfatal-errors
CFLAGS +=$(DEFAULT_OPTIMIZATION) 
CFLAGS +=$(addprefix -I, $(INCLUDES))
CFLAGS +=$(addprefix -D, $(DEFINES))

# Скрипт линкера
#-------------------------------------------------------------------------------
LDSCR_PATH = platform/ld-scripts
LDSCRIPT   = stm32_flash.ld

# Настройки линкера
#-------------------------------------------------------------------------------
LDFLAGS += -nostartfiles
LDFLAGS += -mthumb 
LDFLAGS += -Wl,--gc-sections   # /Не линкуем неиспользуемый код и данные
LDFLAGS += -specs=nano.specs
LDFLAGS += -L$(LDSCR_PATH)
LDFLAGS += -T$(LDSCR_PATH)/$(LDSCRIPT)
LDFLAGS += $(addprefix -L, $(LIBPATH))
LDFLAGS += $(LIBS)

# Настройки ассемблера
#-------------------------------------------------------------------------------
AFLAGS += -alh
AFLAGS += -mapcs-32

# Список объектных файлов
#-------------------------------------------------------------------------------
OBJS += $(patsubst %.c, %.o, $(wildcard  $(addsuffix /*.c, $(SOURCEDIRS))))
OBJS += $(patsubst %.s, %.o, $(STARTUP))


# Пути поиска make
#-------------------------------------------------------------------------------
VPATH := $(SOURCEDIRS)

# Список файлов к удалению командой "make clean"
#-------------------------------------------------------------------------------
TOREMOVE += *.elf *.hex
TOREMOVE += $(addsuffix /*.o, $(SOURCEDIRS))
TOREMOVE += $(addsuffix /*.d, $(SOURCEDIRS))
TOREMOVE += $(STDPERIPH_SRC_PATH)/*.o
TOREMOVE += $(patsubst %.s, %.o, $(STARTUP))
TOREMOVE += $(TARGET)

# Собрать все
#-------------------------------------------------------------------------------
all: $(TARGET).hex size 

# Очистка
#-------------------------------------------------------------------------------
clean:
	@$(RM) -f $(TOREMOVE)  

# Создание .hex файла
#-------------------------------------------------------------------------------
$(TARGET).hex: $(TARGET).elf
	@$(CP) -Oihex $(EXE_DIR)/$(TARGET).elf $(EXE_DIR)/$(TARGET).hex
        
# Показываем размер
#-------------------------------------------------------------------------------
size:
	@echo "Application: $(USER_APP)"
	@echo "---------------------------------------------------"
	@$(SZ) $(EXE_DIR)/$(TARGET).elf
	@echo "---------------------------------------------------"
	@echo "ASM flags: $(AFLAGS)"
	@echo "---------------------------------------------------"
	@echo "GCC flags: $(CFLAGS)" 
	@echo "---------------------------------------------------"
	@echo "DEBUG: $(DEBUG)"
	@echo "ASSERT: $(ASSERT)"
	@echo "LOG: $(LOG)" 

# Линковка
#------------------------------------------------------------------------------- 
$(TARGET).elf: $(OBJS)
	@$(LD) $(LDFLAGS) $^ -o $(EXE_DIR)/$@

OPT = $(notdir $@)

# Компиляция
#------------------------------------------------------------------------------- 
%.o: %.c
	@echo "$<"
	@$(CC) $(CFLAGS) -c $< -o $@
 
%.o: %.s
	@echo "$<"
	@echo "OPTIMIZATION: $(AFLAGS)"
	@$(AS) $(AFLAGS) -c $< -o $@ > /dev/null

# Сгенерированные gcc зависимости
#-------------------------------------------------------------------------------
include $(wildcart *.d)

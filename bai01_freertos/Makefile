TARGET = main

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

CPU = cortex-m3

# ==========================================================
# Compiler flags
# ==========================================================

CFLAGS = -mcpu=$(CPU) \
         -mthumb \
         -O0 \
         -g3 \
         -Wall \
         -Wextra \
         -std=c11 \
         -ffunction-sections \
         -fdata-sections

# ==========================================================
# Include paths
# ==========================================================

CFLAGS += -I.
CFLAGS += -IFreeRTOS/include
CFLAGS += -IFreeRTOS/portable/GCC/ARM_CM3

CFLAGS += -I../STM32CubeF1/Drivers/CMSIS/Include
CFLAGS += -I../STM32CubeF1/Drivers/CMSIS/Device/ST/STM32F1xx/Include

# ==========================================================
# Linker flags
# ==========================================================

LDFLAGS = -mcpu=$(CPU) \
          -mthumb \
          -T STM32F103C8TX_FLASH.ld \
          -Wl,--gc-sections \
          -Wl,-Map=$(TARGET).map \
          --specs=nano.specs \
          --specs=nosys.specs

# ==========================================================
# FreeRTOS source files
# ==========================================================

C_SOURCES = \
    main.c \
    FreeRTOS/tasks.c \
    FreeRTOS/queue.c \
    FreeRTOS/list.c \
    FreeRTOS/portable/GCC/ARM_CM3/port.c \
    FreeRTOS/portable/MemMang/heap_4.c

# ==========================================================
# Assembly
# ==========================================================

ASM_SOURCES = \
    startup_stm32f103xb.s

# ==========================================================
# Object files
# ==========================================================

OBJECTS = \
    $(C_SOURCES:.c=.o) \
    $(ASM_SOURCES:.s=.o)

# ==========================================================
# Default target
# ==========================================================

all: $(TARGET).elf $(TARGET).bin


# ==========================================================
# ELF
# ==========================================================

$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@


# ==========================================================
# BIN
# ==========================================================

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@


# ==========================================================
# Compile C
# ==========================================================

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# ==========================================================
# Compile Assembly
# ==========================================================

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@


# ==========================================================
# Clean
# ==========================================================

clean:
	rm -f $(OBJECTS)
	rm -f $(TARGET).elf
	rm -f $(TARGET).bin
	rm -f $(TARGET).map

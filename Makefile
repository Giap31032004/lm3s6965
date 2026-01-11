# ====================================================================
# PROJECT CONFIGURATION
# ====================================================================
TARGET = kernel

# 1. Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# 2. Compiler Flags
CFLAGS  = -mcpu=cortex-m3 -mthumb -O2 -ffreestanding -nostdlib -g -Wall -std=c99

# --- INCLUDE PATHS (Thêm đường dẫn đến folder tasks để tìm task.h) ---
CFLAGS += -I./include
CFLAGS += -I./kernel/include
CFLAGS += -I./drivers/serial
CFLAGS += -I./drivers/timer
CFLAGS += -I./drivers/gpio
CFLAGS += -I./drivers/i2c
CFLAGS += -I./drivers/dma
CFLAGS += -I./app        
CFLAGS += -I./app/tasks  # <--- [MỚI] Thêm dòng này để include "task.h" từ folder tasks

# 3. Linker Flags
LDSCRIPT = bsp/lm3s6965/linker.ld
LDFLAGS  = -T $(LDSCRIPT) -nostdlib

# ====================================================================
# SOURCE FILE MAPPING (VPATH & SOURCES)
# ====================================================================

# 4. VPATH: Chỉ dẫn cho Make biết tìm file .c ở đâu
VPATH += arch/arm_cm3
VPATH += kernel/core kernel/ipc kernel/mem kernel/algo
VPATH += drivers/serial drivers/timer drivers/gpio drivers/i2c drivers/dma
VPATH += app             # Để tìm main.c
VPATH += app/tasks       # <--- [QUAN TRỌNG] Thêm dòng này để tìm app_global.c, shell.c...

# 5. Source Files

# --- APP FILES ---
SRCS_C  = main.c
SRCS_C += app_global.c
SRCS_C += shell.c
SRCS_C += sensor.c
SRCS_C += deadlock.c
# Lưu ý: Nếu file "task" trong list của bạn là "task.c" thì bỏ comment dòng dưới:
# SRCS_C += task.c

# --- KERNEL CORE ---
SRCS_C += syscalls.c scheduler.c task_manage.c timer.c utils.c

# --- KERNEL IPC & ALGO ---
SRCS_C += queue.c sync.c ipc.c 
SRCS_C += banker.c 
SRCS_C += heap.c

# --- DRIVERS & ARCH ---
SRCS_C += uart_lm3s.c systick.c mpu.c
SRCS_C += gpio.c i2c.c dma.c

# --- ASSEMBLY FILES ---
SRCS_S  = startup.s context.s

# Tự động tạo danh sách file .o
OBJS = $(SRCS_C:.c=.o) $(SRCS_S:.s=.o)

# ====================================================================
# BUILD RULES
# ====================================================================

all: $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

run:
	qemu-system-arm -M lm3s6965evb -kernel $(TARGET).bin -serial mon:stdio -nographic

clean:
	rm -f *.o *.elf *.bin
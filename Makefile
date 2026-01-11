# ====================================================================
# PROJECT CONFIGURATION
# ====================================================================
TARGET = kernel
# (Hoặc để là myos nếu bạn thích)

# 1. Toolchain
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# 2. Compiler Flags (Tương đương CFLAGS cũ nhưng thêm đường dẫn Header)
# -mcpu=cortex-m3 -mthumb -O2 -g -Wall: Giữ nguyên từ makefile cũ
# -I...: Thêm các thư mục chứa file .h để trình biên dịch tìm thấy
CFLAGS  = -mcpu=cortex-m3 -mthumb -O2 -ffreestanding -nostdlib -g -Wall -std=c99
CFLAGS += -I./include
CFLAGS += -I./kernel/include
CFLAGS += -I./drivers/serial
CFLAGS += -I./drivers/timer
CFLAGS += -I./drivers/gpio
CFLAGS += -I./drivers/i2c
CFLAGS += -I./drivers/dma

# 3. Linker Flags (Tương đương LDFLAGS cũ)
# Cập nhật đường dẫn file linker.ld (đã chuyển vào bsp)
LDSCRIPT = bsp/lm3s6965/linker.ld
LDFLAGS  = -T $(LDSCRIPT) -nostdlib

# ====================================================================
# SOURCE FILE MAPPING (VPATH & SOURCES)
# ====================================================================

# 4. VPATH: Chỉ dẫn cho Make biết file nằm ở thư mục nào
# (Thay vì phải gõ kernel/core/process.c, chỉ cần gõ process.c)
VPATH += arch/arm_cm3
VPATH += kernel/core kernel/ipc kernel/mem kernel/algo
VPATH += drivers/serial drivers/timer drivers/gpio
VPATH += app app/tasks
VPATH += drivers/serial drivers/timer drivers/gpio drivers/i2c
VPATH += drivers/serial drivers/timer drivers/gpio drivers/i2c drivers/dma
# 5. Source Files (Ánh xạ từ danh sách SRC cũ sang tên/file mới)

# --- APP ---
SRCS_C  = main.c
SRCS_C += task.c          # (Nằm trong app/tasks)

# --- KERNEL CORE ---
SRCS_C += syscalls.c      # (Code xử lý svc)

# --- KERNEL IPC & ALGO ---
SRCS_C += queue.c sync.c ipc.c 
SRCS_C += banker.c 
SRCS_C += heap.c          # (Ánh xạ từ memory.c cũ - Kiểm tra xem bạn đã đổi tên chưa, nếu chưa thì để memory.c)

# --- DRIVERS & ARCH ---
SRCS_C += uart_lm3s.c     # (Ánh xạ từ uart.c cũ - đã đổi tên)
SRCS_C += systick.c 
SRCS_C += mpu.c

SRCS_C += scheduler.c 
SRCS_C += task_manage.c 
SRCS_C += timer.c 
SRCS_C += utils.c
SRCS_C += gpio.c
SRCS_C += i2c.c
SRCS_C += dma.c
# --- UTILS (Sửa lỗi memset) ---
# (Bạn tạo file này chứa hàm memset như hướng dẫn trước, hoặc viết thẳng vào kernel.c thì bỏ dòng này)
# SRCS_C += utils.c      

# --- ASSEMBLY FILES ---
SRCS_S  = startup.s 
SRCS_S += context.s       # (Ánh xạ từ context_switch.s cũ - đã đổi tên)

# Tự động tạo danh sách file .o
OBJS = $(SRCS_C:.c=.o) $(SRCS_S:.s=.o)

# ====================================================================
# BUILD RULES (Giữ nguyên logic cũ)
# ====================================================================

all: $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

# Quy tắc biên dịch file .c (Make tự tìm file nhờ VPATH)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Quy tắc biên dịch file .s
%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

run:
	qemu-system-arm -M lm3s6965evb -kernel $(TARGET).bin -serial mon:stdio -nographic

clean:
	rm -f *.o *.elf *.bin
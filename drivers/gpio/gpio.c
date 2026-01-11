

#include "gpio.h"
#include "kernel.h" // Để dùng OS_ENTER_CRITICAL

/* --- ĐỊNH NGHĨA THANH GHI (OFFSETS) --- */
// System Control (Để bật Clock cho GPIO)
#define SYSCTL_RCGC2        (*(volatile uint32_t *)0x400FE108)

// Offsets của GPIO Register
#define GPIO_DATA_OFFSET    0x3FC // Đọc/Ghi dữ liệu (Masked Access tối đa)
#define GPIO_DIR_OFFSET     0x400 // Hướng (Direction)
#define GPIO_AFSEL_OFFSET   0x420 // Alternate Function (Chức năng thay thế)
#define GPIO_DEN_OFFSET     0x51C // Digital Enable (Kích hoạt số)

/* --- HÀM TRỢ GIÚP: Lấy bit Clock dựa trên địa chỉ Port --- */
static uint32_t get_rcgc2_mask(uint32_t port_base) {
    switch(port_base) {
        case GPIO_PORTA_BASE: return (1 << 0);
        case GPIO_PORTB_BASE: return (1 << 1);
        case GPIO_PORTC_BASE: return (1 << 2);
        case GPIO_PORTD_BASE: return (1 << 3);
        case GPIO_PORTE_BASE: return (1 << 4);
        case GPIO_PORTF_BASE: return (1 << 5);
        case GPIO_PORTG_BASE: return (1 << 6);
        default: return 0;
    }
}

/* --- HÀM KHỞI TẠO --- */
void gpio_init(uint32_t port_base, uint8_t pin_mask, uint8_t direction) {
    OS_ENTER_CRITICAL();

    /* 1. Bật Clock cho Port tương ứng */
    uint32_t rcgc_mask = get_rcgc2_mask(port_base);
    SYSCTL_RCGC2 |= rcgc_mask;

    /* 2. Delay nhỏ để Clock ổn định (bắt buộc với chip thực) */
    volatile int delay;
    for(delay = 0; delay < 100; delay++);

    /* 3. Truy cập các thanh ghi thông qua Port Base */
    volatile uint32_t *reg_dir = (volatile uint32_t *)(port_base + GPIO_DIR_OFFSET);
    volatile uint32_t *reg_den = (volatile uint32_t *)(port_base + GPIO_DEN_OFFSET);
    volatile uint32_t *reg_afsel = (volatile uint32_t *)(port_base + GPIO_AFSEL_OFFSET);

    /* 4. Cấu hình Hướng (Direction) */
    if (direction == GPIO_DIR_OUTPUT) {
        *reg_dir |= pin_mask; // Set bit thành 1 là Output
    } else {
        *reg_dir &= ~pin_mask; // Clear bit thành 0 là Input
    }

    /* 5. Tắt chức năng thay thế (Để dùng làm GPIO thường) */
    *reg_afsel &= ~pin_mask;

    /* 6. Bật chức năng Digital (Digital Enable) */
    *reg_den |= pin_mask;

    OS_EXIT_CRITICAL();
}

/* --- HÀM GHI (WRITE) --- */
void gpio_write(uint32_t port_base, uint8_t pin_mask, uint8_t value) {
    // Địa chỉ thanh ghi DATA (Offset 0x3FC cho phép truy cập tất cả 8 pin)
    volatile uint32_t *reg_data = (volatile uint32_t *)(port_base + GPIO_DATA_OFFSET);
    
    // GPIO thao tác bit nên cần Critical Section để tránh race condition
    OS_ENTER_CRITICAL();
    
    if (value) {
        *reg_data |= pin_mask; // Bật lên 1
    } else {
        *reg_data &= ~pin_mask; // Tắt về 0
    }
    
    OS_EXIT_CRITICAL();
}

/* --- HÀM ĐẢO (TOGGLE) --- */
void gpio_toggle(uint32_t port_base, uint8_t pin_mask) {
    volatile uint32_t *reg_data = (volatile uint32_t *)(port_base + GPIO_DATA_OFFSET);
    
    OS_ENTER_CRITICAL();
    *reg_data ^= pin_mask; // Phép XOR để đảo bit
    OS_EXIT_CRITICAL();
}

/* --- HÀM ĐỌC (READ) --- */
uint32_t gpio_read(uint32_t port_base, uint8_t pin_mask) {
    volatile uint32_t *reg_data = (volatile uint32_t *)(port_base + GPIO_DATA_OFFSET);
    return (*reg_data) & pin_mask;
}
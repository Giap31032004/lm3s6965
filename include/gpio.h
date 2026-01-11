#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/* --- ĐỊNH NGHĨA ĐỊA CHỈ BASE CỦA CÁC PORT (LM3S Memory Map) --- */
#define GPIO_PORTA_BASE     0x40004000
#define GPIO_PORTB_BASE     0x40005000
#define GPIO_PORTC_BASE     0x40006000
#define GPIO_PORTD_BASE     0x40007000
#define GPIO_PORTE_BASE     0x40024000
#define GPIO_PORTF_BASE     0x40025000 
#define GPIO_PORTG_BASE     0x40026000

/* --- ĐỊNH NGHĨA CÁC PIN (Mặt nạ bit) --- */
#define GPIO_PIN_0          (1U << 0)
#define GPIO_PIN_1          (1U << 1)
#define GPIO_PIN_2          (1U << 2)
#define GPIO_PIN_3          (1U << 3)
#define GPIO_PIN_4          (1U << 4)
#define GPIO_PIN_5          (1U << 5)
#define GPIO_PIN_6          (1U << 6)
#define GPIO_PIN_7          (1U << 7)

/* --- HƯỚNG DỮ LIỆU --- */
#define GPIO_DIR_INPUT      0
#define GPIO_DIR_OUTPUT     1

/* --- PUBLIC FUNCTIONS --- */

/**
 * @brief Khởi tạo một chân GPIO
 * @param port_base: Địa chỉ cơ sở của Port (Ví dụ: GPIO_PORTF_BASE)
 * @param pin_mask: Các pin cần init (Ví dụ: GPIO_PIN_0 | GPIO_PIN_1)
 * @param direction: GPIO_DIR_INPUT hoặc GPIO_DIR_OUTPUT
 */
void gpio_init(uint32_t port_base, uint8_t pin_mask, uint8_t direction);

/**
 * @brief Ghi mức logic (0 hoặc 1) ra chân GPIO
 * @param value: 0 (Low) hoặc 1 (High)
 */
void gpio_write(uint32_t port_base, uint8_t pin_mask, uint8_t value);

/**
 * @brief Đảo trạng thái chân GPIO (Đang 0 thành 1, đang 1 thành 0)
 */
void gpio_toggle(uint32_t port_base, uint8_t pin_mask);

/**
 * @brief Đọc giá trị hiện tại của chân GPIO
 * @return 0 hoặc 1 (nếu đọc 1 pin), hoặc giá trị mask (nếu đọc nhiều pin)
 */
uint32_t gpio_read(uint32_t port_base, uint8_t pin_mask);

#endif
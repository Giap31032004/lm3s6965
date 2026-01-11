#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include <stdbool.h>

/* --- CẤU HÌNH --- */
#define I2C_MASTER_BASE     0x40020000 // Địa chỉ I2C0 Master

/* --- PUBLIC FUNCTIONS --- */

/**
 * @brief Khởi tạo I2C0 (PB2 = SCL, PB3 = SDA) tốc độ 100kbps
 */
void i2c_init(void);

/**
 * @brief Ghi 1 byte dữ liệu vào thanh ghi của Slave
 * @param slave_addr: Địa chỉ 7-bit của thiết bị (VD: 0x3C cho OLED)
 * @param reg_addr: Địa chỉ thanh ghi cần ghi
 * @param data: Dữ liệu cần ghi
 * @return true nếu thành công, false nếu lỗi
 */
bool i2c_write_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t data);

/**
 * @brief Đọc 1 byte dữ liệu từ thanh ghi của Slave
 * @param slave_addr: Địa chỉ 7-bit của thiết bị
 * @param reg_addr: Địa chỉ thanh ghi muốn đọc
 * @param data: Con trỏ để lưu dữ liệu đọc được
 * @return true nếu thành công
 */
bool i2c_read_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data);

#endif
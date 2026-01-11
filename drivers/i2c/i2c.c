#include "i2c.h"
#include "kernel.h" // Để dùng Mutex
#include "gpio.h"   // Để cấu hình chân GPIO
#include "sync.h"   // <--- THÊM DÒNG NÀY (Để dùng os_mutex_t)

/* --- THANH GHI I2C (LM3S6965) --- */
#define I2C_MSA    (*(volatile uint32_t *)(I2C_MASTER_BASE + 0x000)) // Master Slave Address
#define I2C_MCS    (*(volatile uint32_t *)(I2C_MASTER_BASE + 0x004)) // Master Control/Status
#define I2C_MDR    (*(volatile uint32_t *)(I2C_MASTER_BASE + 0x008)) // Master Data
#define I2C_MTPR   (*(volatile uint32_t *)(I2C_MASTER_BASE + 0x00C)) // Master Timer Period (Speed)
#define I2C_MCR    (*(volatile uint32_t *)(I2C_MASTER_BASE + 0x020)) // Master Configuration

/* --- THANH GHI HỆ THỐNG & GPIO --- */
#define SYSCTL_RCGC1    (*(volatile uint32_t *)0x400FE104) // Legacy Clock Control
#define SYSCTL_RCGC2    (*(volatile uint32_t *)0x400FE108) // GPIO Clock
#define GPIO_PORTB_BASE 0x40005000
#define GPIO_AFSEL      (*(volatile uint32_t *)(GPIO_PORTB_BASE + 0x420))
#define GPIO_ODR        (*(volatile uint32_t *)(GPIO_PORTB_BASE + 0x50C)) // Open Drain
#define GPIO_DEN        (*(volatile uint32_t *)(GPIO_PORTB_BASE + 0x51C))

/* --- CỜ ĐIỀU KHIỂN (CONTROL COMMANDS) --- */
#define CMD_RUN          0x01
#define CMD_START        0x02
#define CMD_STOP         0x04
#define CMD_ACK          0x08

/* --- CỜ TRẠNG THÁI (STATUS FLAGS) --- */
#define STS_BUSY         0x01
#define STS_ERROR        0x02

/* --- MUTEX BẢO VỆ BUS --- */
static os_mutex_t i2c_mutex;

/* --- HÀM PHỤ TRỢ: CHỜ BUS RẢNH --- */
static bool i2c_wait_busy(void) {
    /* Chờ cho đến khi bit BUSY (bit 0) về 0 */
    /* Trong OS thực tế, nên dùng Timeout để tránh treo vĩnh viễn */
    int timeout = 100000;
    while (I2C_MCS & STS_BUSY) {
        if (--timeout == 0) return false; // Time out
    }
    
    /* Kiểm tra xem có lỗi không (Bit Error) */
    if (I2C_MCS & STS_ERROR) return false;
    return true;
}

/* --- KHỞI TẠO --- */
void i2c_init(void) {
    /* 1. Init Mutex */
    mutex_init(&i2c_mutex);

    OS_ENTER_CRITICAL();

    /* 2. Cấp Clock cho I2C0 và GPIO Port B */
    SYSCTL_RCGC1 |= (1 << 12); // Enable I2C0
    SYSCTL_RCGC2 |= (1 << 1);  // Enable Port B
    
    // Delay nhỏ
    volatile int i; for(i=0; i<100; i++);

    /* 3. Cấu hình GPIO PB2 (SCL) và PB3 (SDA) */
    // Bật chức năng thay thế (Alternate Function)
    GPIO_AFSEL |= (1<<2) | (1<<3); 
    
    // Cực kỳ quan trọng: I2C SDA (PB3) phải là Open-Drain
    GPIO_ODR |= (1<<3); 
    
    // Bật Digital
    GPIO_DEN |= (1<<2) | (1<<3);

    /* 4. Khởi tạo I2C Master */
    I2C_MCR = 0x10; // Enable Master function

    /* 5. Cài đặt tốc độ 100kbps 
       Công thức: TPR = (System Clock / (2 * (SCL_LP + SCL_HP) * SCL_CLK)) - 1
       Với 50MHz, 100kbps -> Giá trị khoảng 0x18 (24)
    */
    I2C_MTPR = 0x18; 

    OS_EXIT_CRITICAL();
}

/* --- GHI 1 BYTE --- */
bool i2c_write_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t data) {
    mutex_lock(&i2c_mutex); // Chiếm quyền Bus

    /* Bước 1: Gửi địa chỉ thanh ghi (Register Address) */
    I2C_MSA = (slave_addr << 1) | 0; // Bit 0 = 0 (Write)
    I2C_MDR = reg_addr;
    
    // START + RUN
    I2C_MCS = CMD_START | CMD_RUN; 
    if (!i2c_wait_busy()) { mutex_unlock(&i2c_mutex); return false; }

    /* Bước 2: Gửi dữ liệu (Data) */
    I2C_MDR = data;
    
    // RUN + STOP
    I2C_MCS = CMD_RUN | CMD_STOP;
    if (!i2c_wait_busy()) { mutex_unlock(&i2c_mutex); return false; }

    mutex_unlock(&i2c_mutex); // Trả quyền Bus
    return true;
}

/* --- ĐỌC 1 BYTE --- */
bool i2c_read_byte(uint8_t slave_addr, uint8_t reg_addr, uint8_t *data) {
    mutex_lock(&i2c_mutex);

    /* Bước 1: Ghi địa chỉ thanh ghi muốn đọc (Dummy Write) */
    I2C_MSA = (slave_addr << 1) | 0; // Write mode
    I2C_MDR = reg_addr;
    
    // START + RUN (Chưa STOP vội)
    I2C_MCS = CMD_START | CMD_RUN;
    if (!i2c_wait_busy()) { mutex_unlock(&i2c_mutex); return false; }

    /* Bước 2: Đọc dữ liệu về */
    I2C_MSA = (slave_addr << 1) | 1; // Bit 0 = 1 (Read mode)
    
    // Repeated START + RUN + STOP + ACK
    I2C_MCS = CMD_START | CMD_RUN | CMD_STOP | CMD_ACK; 
    if (!i2c_wait_busy()) { mutex_unlock(&i2c_mutex); return false; }

    *data = (uint8_t)I2C_MDR; // Lấy dữ liệu từ thanh ghi

    mutex_unlock(&i2c_mutex);
    return true;
}
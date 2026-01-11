#include "dma.h"
#include "kernel.h" // Chứa OS_ENTER_CRITICAL

/* --- ĐỊNH NGHĨA THANH GHI (Chỉ file .c mới cần biết mấy cái này) --- */
#define DMA_BASE            0x400FF000

#define DMA_STAT        (*(volatile uint32_t *)(DMA_BASE + 0x000))
#define DMA_CFG         (*(volatile uint32_t *)(DMA_BASE + 0x004))
#define DMA_CTLBASE     (*(volatile uint32_t *)(DMA_BASE + 0x008))
#define DMA_SWREQ       (*(volatile uint32_t *)(DMA_BASE + 0x014))
#define DMA_ENASET      (*(volatile uint32_t *)(DMA_BASE + 0x028))

#define SYSCTL_RCGC2    (*(volatile uint32_t *)0x400FE108)

/* --- CẤU HÌNH CONTROL WORD --- */
#define UDMA_DST_INC_8   (0x0 << 30)
#define UDMA_SRC_INC_8   (0x0 << 26)
#define UDMA_SIZE_8      (0x0 << 24)
#define UDMA_ARB_1024    (0xF << 14)
#define UDMA_MODE_AUTO   (0x2 << 0)

/* --- BẢNG ĐIỀU KHIỂN --- */
// Phải align 1024 bytes (Yêu cầu phần cứng)
dma_control_entry_t dma_table[64] __attribute__((aligned(1024)));
#define DMA_CH_SW  30

/* --- IMPLEMENTATION (CODE THỰC THI NẰM Ở ĐÂY) --- */

void dma_init(void) {
    OS_ENTER_CRITICAL();

    /* 1. Bật Clock DMA */
    SYSCTL_RCGC2 |= (1 << 13);
    volatile int i; for(i=0; i<100; i++);

    /* 2. Bật Master Enable */
    DMA_CFG = 1;

    /* 3. Trỏ Base Address vào bảng dma_table */
    DMA_CTLBASE = (uint32_t)&dma_table;

    OS_EXIT_CRITICAL();
}

bool dma_memcpy(void *src, void *dst, uint32_t size) {
    if (size == 0 || size > 1024) return false;

    OS_ENTER_CRITICAL();

    dma_control_entry_t *ch = &dma_table[DMA_CH_SW];

    /* Tính toán End Pointer */
    ch->src_end_ptr = (void *)((uint32_t)src + size - 1);
    ch->dst_end_ptr = (void *)((uint32_t)dst + size - 1);

    /* Cấu hình Control Word */
    ch->control = UDMA_DST_INC_8 | UDMA_SRC_INC_8 | UDMA_SIZE_8 |
                  UDMA_ARB_1024  | UDMA_MODE_AUTO |
                  ((size - 1) << 4);

    /* Kích hoạt */
    DMA_ENASET = (1 << DMA_CH_SW);
    DMA_SWREQ = (1 << DMA_CH_SW);

    OS_EXIT_CRITICAL();
    return true;
}
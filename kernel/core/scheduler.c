#include "kernel.h"
#include "queue.h"
#include "uart.h"
#include "mpu.h"

#define SCB_ICSR (*(volatile uint32_t*)0xE000ED04)
#define PENDSVSET_BIT (1UL << 28)

/* Biến nội bộ của Scheduler */
queue_t ready_queue[MAX_PRIORITY];
uint32_t top_ready_priority_bitmap = 0;

/* Biến toàn cục (định nghĩa thực sự) */
PCB_t *current_pcb = NULL;
PCB_t *next_pcb = NULL;

extern void start_first_task(uint32_t *first_sp); // Từ assembly

void add_task_to_ready_queue(PCB_t *p) {
    uint8_t prio = p->dynamic_priority;
    if(prio >= MAX_PRIORITY) prio = MAX_PRIORITY - 1;

    queue_enqueue(&ready_queue[prio], p);
    top_ready_priority_bitmap |= (1UL << prio);
}

PCB_t* get_highest_priority_ready_task() {
    if (top_ready_priority_bitmap == 0) return NULL;
    
    // Tối ưu dùng __builtin_clz
    int highest_prio = 31 - __builtin_clz(top_ready_priority_bitmap);
    if (highest_prio >= MAX_PRIORITY) highest_prio = MAX_PRIORITY - 1;

    PCB_t *p = queue_dequeue(&ready_queue[highest_prio]);
    
    if(queue_is_empty(&ready_queue[highest_prio])) {
        top_ready_priority_bitmap &= ~(1UL << highest_prio);
    }
    uart_print("Selected process use priority ");
    uart_print_dec(highest_prio); // Cần thêm hàm này vào uart.h
    uart_print("\r\n");
    return p;
}

void process_schedule(void) {
    /* 1. Vào vùng găng để tránh ngắt chen ngang lúc tính toán */
    OS_ENTER_CRITICAL();

    /* 2. Kiểm tra xem có task nào trong hàng đợi không */
    if (top_ready_priority_bitmap == 0) {
        OS_EXIT_CRITICAL();
        return; // Không có việc gì làm -> Về (hoặc chạy Idle Task)
    }

    /* 3. Lấy task có độ ưu tiên cao nhất */
    PCB_t *pnext = get_highest_priority_ready_task();
    if (!pnext) {
        OS_EXIT_CRITICAL(); 
        return;
    }

    /* 4. Xử lý Task hiện tại (Nếu đang chạy thì đẩy về Ready) */
    if (current_pcb != NULL && current_pcb->state == PROC_RUNNING) {
        current_pcb->state = PROC_READY;
        add_task_to_ready_queue(current_pcb);
    }

    /* 5. Cấu hình Task mới */
    // Chỉ cập nhật biến toàn cục next_pcb để PendSV dùng
    next_pcb = pnext;      
    pnext->state = PROC_RUNNING;

    /* [QUAN TRỌNG] Cấu hình MPU cho Task sắp chạy 
       Phải làm lúc này vì khi nhảy sang Task mới là MPU phải sẵn sàng rồi 
    */
    mpu_config_for_task(pnext);

    uart_print("Switching to PID: ");
    uart_print_dec(pnext->pid);
    uart_print("\r\n");

    /* 6. Phân loại chuyển ngữ cảnh */
    if (current_pcb == NULL) {
        /* TRƯỜNG HỢP 1: Khởi động Task đầu tiên của hệ thống */
        current_pcb = pnext;
        
        // start_first_task sẽ nhảy đi luôn và không bao giờ quay lại đây
        // Nó sẽ tự lo việc bật lại ngắt (thông qua việc set PRIMASK = 0)
        start_first_task(current_pcb->stack_ptr); 
    } 
    else {
        /* TRƯỜNG HỢP 2: Chuyển ngữ cảnh bình thường */
        // Kích hoạt PendSV
        SCB_ICSR |= PENDSVSET_BIT;
        
        // Thoát vùng găng để PendSV có thể xảy ra ngay sau dòng này
        OS_EXIT_CRITICAL();
    }
}

/* Hàm khởi tạo riêng cho scheduler (được gọi bởi process_init) */
void scheduler_init_queues(void) {
    for(int i = 0; i < MAX_PRIORITY; i++) {
        queue_init(&ready_queue[i]);
    }
    top_ready_priority_bitmap = 0;
}
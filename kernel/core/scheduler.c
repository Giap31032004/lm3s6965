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
    OS_ENTER_CRITICAL();

    if (top_ready_priority_bitmap == 0) {
        OS_EXIT_CRITICAL();
        return;
    }

    PCB_t *pnext = get_highest_priority_ready_task();
    if (!pnext) {
        OS_EXIT_CRITICAL(); 
        return;
    }

    if (current_pcb != NULL) {
        if (current_pcb->state == PROC_RUNNING) {
            current_pcb->state = PROC_READY;
            add_task_to_ready_queue(current_pcb);
        }
    }

    pnext->state = PROC_RUNNING;
    mpu_config_for_task(pnext);
    OS_EXIT_CRITICAL();

    uart_print("Switching to process ");
    uart_print_dec(pnext->pid);
    uart_print("\r\n");

    if (current_pcb == NULL) {
        current_pcb = pnext;
        start_first_task(current_pcb->stack_ptr);
    } else {
        next_pcb = pnext;
        SCB_ICSR |= PENDSVSET_BIT;
    }
}

/* Hàm khởi tạo riêng cho scheduler (được gọi bởi process_init) */
void scheduler_init_queues(void) {
    for(int i = 0; i < MAX_PRIORITY; i++) {
        queue_init(&ready_queue[i]);
    }
    top_ready_priority_bitmap = 0;
}
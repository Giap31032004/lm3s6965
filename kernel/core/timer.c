#include "kernel.h"

#define SCB_ICSR (*(volatile uint32_t*)0xE000ED04)
#define PENDSVSET_BIT (1UL << 28)

volatile uint32_t tick_count = 0;

void os_delay(uint32_t ticks) {
    OS_ENTER_CRITICAL();
    current_pcb->wake_up_tick = tick_count + ticks;
    current_pcb->state = PROC_WAITING_TIME;
    OS_EXIT_CRITICAL();

    process_schedule(); // Gọi scheduler để đổi task
    
    // Trigger PendSV để ép context switch ngay lập tức (nếu process_schedule chưa làm)
    SCB_ICSR |= PENDSVSET_BIT;
}

void process_timer_tick(void) {
    tick_count++;
    int need_schedule = 0; 

    for (int i = 0; i < MAX_PROCESSES; i++) {
        PCB_t *p = &pcb_table[i];
        if (p->state == PROC_WAITING_TIME) {
            if (p->wake_up_tick <= tick_count) {
                p->state = PROC_READY;
                p->wake_up_tick = 0;
                add_task_to_ready_queue(p);
                
                if (current_pcb && p->dynamic_priority > current_pcb->dynamic_priority) {
                    need_schedule = 1;
                }
            }
        }
    }

    if (current_pcb && current_pcb->time_slice > 0) {
        current_pcb->time_slice--;
        if (current_pcb->time_slice == 0) {
            current_pcb->time_slice = 5; 
            need_schedule = 1;
        }
    }

    if(need_schedule) {
        SCB_ICSR |= PENDSVSET_BIT;
    }
}
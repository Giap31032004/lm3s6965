#ifndef APP_TASKS_H
#define APP_TASKS_H

#include <stdint.h>
#include "kernel.h"
#include "sync.h"
#include "ipc.h"

/* --- KHAI BÁO BIẾN TOÀN CỤC (EXTERN) --- */
extern volatile int current_temperature;
extern volatile int system_uptime;

extern os_msg_queue_t temp_queue;
extern os_mutex_t app_mutex;
extern os_mutex_t mutex_A;
extern os_mutex_t mutex_B;

/* --- PROTOTYPES CÁC TASK --- */

// Nhóm IoT
void task_sensor_update(void);
void task_display(void);
void task_alarm(void);

// Nhóm System
void task_logger(void);
void task_shell(void);

// Nhóm Test
void task_deadlock_1(void);
void task_deadlock_2(void);
void task_banker1(void);
void task_banker2(void);

#endif // APP_TASKS_H
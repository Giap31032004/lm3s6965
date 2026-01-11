#include "task.h"

/* Định nghĩa thực tế các biến toàn cục */
volatile int current_temperature = 25; 
volatile int system_uptime = 0;

/* Các đối tượng OS này sẽ được init trong main */
os_msg_queue_t temp_queue;
os_mutex_t app_mutex;
os_mutex_t mutex_A;
os_mutex_t mutex_B;
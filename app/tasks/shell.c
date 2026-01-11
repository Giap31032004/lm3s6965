#include "task.h"
#include "uart.h"

/* Hàm phụ trợ cho Shell */
static int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/* TASK 4: LOGGER */
void task_logger(void) {
    int counter = 0;
    while (1) {
        os_delay(10); 
        mutex_lock(&app_mutex);
        uart_print("    >>> [LOGGER] Checking system... Count: ");
        uart_print_dec(counter++);
        uart_print("\r\n");
        mutex_unlock(&app_mutex);
    }
}

/* TASK 5: SHELL */
void task_shell(void) {
    char cmd_buffer[32];
    int cmd_index = 0;

    mutex_lock(&app_mutex);
    uart_print("\r\n[SHELL] Ready. Type 'help' to start.\r\n");
    uart_print("MyOS> ");
    mutex_unlock(&app_mutex);

    while (1) {
        char c = uart_getc();

        mutex_lock(&app_mutex);
        uart_putc(c);
        mutex_unlock(&app_mutex);

        if (c == '\r') {
            mutex_lock(&app_mutex);
            uart_print("\n");
            cmd_buffer[cmd_index] = '\0';

            if (my_strcmp(cmd_buffer, "help") == 0) {
                uart_print("Available commands:\r\n");
                uart_print("  help  : Show this help\r\n");
                uart_print("  temp  : Show current temperature\r\n");
                uart_print("  reboot: Restart system\r\n");
            } 
            else if (my_strcmp(cmd_buffer, "temp") == 0) {
                uart_print("Current Temp: ");
                uart_print_dec(current_temperature);
                uart_print(" C\r\n");
            }
            else if (my_strcmp(cmd_buffer, "reboot") == 0) {
                uart_print("Rebooting...\r\n");
                *(volatile uint32_t*)0xE000ED0C = 0x05FA0004;
            }
            else if (cmd_index > 0) {
                uart_print("Unknown command: ");
                uart_print(cmd_buffer);
                uart_print("\r\n");
            }

            uart_print("MyOS> ");
            mutex_unlock(&app_mutex);
            cmd_index = 0;
        } 
        else {
            if (cmd_index < 31) {
                cmd_buffer[cmd_index++] = c;
            }
        }
    }
}
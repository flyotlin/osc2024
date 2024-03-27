#include "mailbox.h"
#include "shell.h"
#include "string.h"
#include "uart.h"

void shell_start()
{
    uart_puts("# ");

    char line[MAX_GETLINE_LEN];
    char c;
    unsigned int index = 0;
    while (1) {
        c = (char) uart_getc();

        if (c == '\n') {
            line[index++] = '\0';
            uart_puts("\n");

            do_cmd(line);

            index = 0;
            uart_puts("# ");
        } else {
            line[index++] = c;
            uart_send(c);
        }
    }
}

void do_cmd(const char* line)
{
    if (strcmp(line, "help") == 0) {
        uart_puts("help\t: print this help menu\nhello\t: print Hello World!\nreboot\t: reboot the device\n");
    } else if (strcmp(line, "hello") == 0) {
        uart_puts("Hello World!\n");
    } else if (strcmp(line, "reboot") == 0) {
        uart_puts("Reboot!\n");

        unsigned int r;
        r = *PM_RSTS;
        r &= ~0xFFFFFAAA;
        *PM_RSTS = PM_WDOG_MAGIC | r;
        *PM_WDOG = PM_WDOG_MAGIC | 10;
        *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
    } else if (strcmp(line, "board") == 0) {
        get_board_revision();
    } else if (strcmp(line, "arm") == 0) {
        get_arm_memory();
    } else {
        uart_puts("command not found\n");
    }
}
#include "exception.h"
#include "fdt.h"
#include "initramfs.h"
#include "mm.h"
#include "ring_buffer.h"
#include "uart.h"
#include "sched.h"
#include "shell.h"

void main()
{
    init_exception_vectors();
    init_interrupt();
    uart_init();
    *UART_IRQs1 |= (1 << 29);   // TODO: refactor
    uart_init_buffer();

    fdt_traverse(initramfs_callback);
    init_mm();

    uart_puts("\nWelcome to kernel!\n");

    init_sched();

    char c;
    while (1) {
        c = uart_async_getc();
        uart_async_send(c);
        if (c == 'e') {
            break;
        }
    }

    // // TODO: test create multiple threads
    // create_thread(idle, 0);
    // for (int i = 0; i < 10; i++) {
    //     uart_puts("hi 1\n");
    // }
    // schedule();

    // create_thread(shell_start, 0);
    // schedule();

    create_thread(idle, 0);
    schedule();
}

int forked = 0;
void idle()
{
    while (1) {
        // _exec("syscall.img", 0);
        _exec("p1.img", 0);
        // _exec("syscall.img", 0);
        // for (int i = 0; i < 10; i++) {
        //     // uart_puts("hi 2\n");
        //     uart_puts("tid: ");
        //     uart_putints(get_current_thread()->tid);
        //     uart_puts("\n");
        // }
        // if (forked == 1) {
        //     _exec("p1.img", 0);
        // }
        // if (forked == 0) {
        //     forked = 1;
        //     // create_thread(idle2, 0);
        // }
        kill_zombies();
        // schedule();
    }
}

int forked2 = 0;
int c = 0;
void idle2()
{
    while (1) {
        if (c == 10000) {
            _exit();
        }
        for (int i = 0; i < 5; i++) {
            // uart_puts("hi 3\n");
            uart_puts("tid: ");
            uart_putints(get_current_thread()->tid);
            uart_puts("\n");
            c += 1;
        }
        if (forked2 == 0) {
            forked2 = 1;
            create_thread(idle, 0);
        }
        schedule();
    }
}
#include "exception.h"
#include "uart.h"

void init_exception_vectors(void)
{
    // set EL1 exception vector table
    asm("adr x0, exception_vector_table");
    asm("msr vbar_el1, x0");
}

void init_interrupt(void)
{
    _init_core_timer();
}

void _init_core_timer(void)
{
    // enable timer
    asm("mov x0, 1");
    asm("msr cntp_ctl_el0, x0");

    // set expired time
    asm("mov x0, %0" : : "r"(EXPIRE_PERIOD));
    asm("msr cntp_tval_el0, x0");

    // unmask timer interrupt
    asm("mov x0, 2");
    // asm("ldr x1, =%0" : : "r"(CORE0_TIMER_IRQ_CTRL));
    asm("ldr x1, =0x40000040");
    asm("str x0, [x1]");
}

void handle_exception(void)
{
}

void handle_interrupt(void)
{
    // seconds after booting
    long count;
    long freq; //62500000 (62.5 MHz)

    asm("mrs %0, cntpct_el0" : "=r"((long) count));
    asm("mrs %0, cntfrq_el0" : "=r"((long) freq));

    uart_putints(count/freq);
    uart_puts(" seconds after booting...\n");

    // set next timeout
    asm("mov x0, 0xfffffff");
    asm("msr cntp_tval_el0, x0");
}

void el2_to_el1(void)
{
    asm("mov     x0, (1 << 31)");
    asm("msr     hcr_el2, x0");
    asm("mov     x0, 0x3c5");
    asm("msr     spsr_el2, x0");
    asm("msr     elr_el2, lr");
    asm("eret");
}

void el1_to_el0(void)
{
    // REF: https://gcc.gnu.org/onlinedocs/gcc/extensions-to-the-c-language-family/how-to-use-inline-assembly-language-in-c-code.html#extended-asm-assembler-instructions-with-c-expression-operands
    // Output/Input Operands
    uart_puts("EL1 to EL0\n");

    asm("mov x0, 0");   // enable interrupt in EL0
    asm("msr spsr_el1, x0");                                // state
    asm("msr elr_el1, %0" : : "r" (&__userspace_start));    // return address
    asm("msr sp_el0, %0" : : "r" (&__userspace_end));       // stack pointer
    asm("eret");
}
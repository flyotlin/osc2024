#include "mailbox.h"
#include "uart.h"

int mailbox_exec(unsigned int channel, unsigned int *msg)
{
    unsigned int r = ((unsigned int)((unsigned long)msg & ~(0xF)) | (channel & 0xF));

    while (*MAILBOX_STATUS & MAILBOX_FULL) {
        asm volatile("nop");
    }

    *MAILBOX_WRITE = r;

    while (*MAILBOX_STATUS & MAILBOX_EMPTY) {
        asm volatile("nop");
    }
    while (r != *MAILBOX_READ) {
        asm volatile("nop");
    }
    return msg[1] == MAILBOX_RESPONSE;
}

int _mbox_exec(trapframe_t *trapframe, unsigned int channel, unsigned int *msg)
{
    unsigned int r = ((unsigned int)((unsigned long)msg & ~(0xF)) | (channel & 0xF));

    do {
        asm volatile("nop");
    } while (*MAILBOX_STATUS & MAILBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MAILBOX_WRITE = r;
    /* now wait for the response */
    while (1) {
        /* is there a response? */
        do {
            asm volatile("nop");
        } while (*MAILBOX_STATUS & MAILBOX_EMPTY);
        /* is it a response to our message? */
        if (r == *MAILBOX_READ) {
            /* is it a valid successful response? */
            // return value pass by x0
            trapframe->x0 = (msg[1] == MAILBOX_RESPONSE);
            return msg[1] == MAILBOX_RESPONSE;
        }
    }
    trapframe->x0 = 0;
    return 0;
}

void get_board_revision(const char *line)
{
    volatile unsigned int __attribute__((aligned(16))) msg[7];
    msg[0] = 7*4;
    msg[1] = MAILBOX_REQUEST;
    msg[2] = MAILBOX_GET_BOARD_REVISION;
    msg[3] = 4;
    msg[4] = MAILBOX_BUF_REQUEST;
    msg[5] = 0;
    msg[6] = MAILBOX_END_TAG;

    if (mailbox_exec(MAILBOX_CH_ARM2VC, msg)) {
        uart_puts("board revision:\n");
        uart_hex(msg[5]);
        uart_puts("\n");
    } else {
        uart_puts("Failed\n");
    }
}

void get_arm_memory(const char *line)
{
    volatile unsigned int  __attribute__((aligned(16))) msg[10];
    msg[0] = 8*4;
    msg[1] = MAILBOX_REQUEST;
    msg[2] = MAILBOX_GET_ARM_MEMORY;
    msg[3] = 8;
    msg[4] = MAILBOX_BUF_REQUEST;
    msg[5] = 0;
    msg[6] = 0;
    msg[7] = MAILBOX_END_TAG;

    if (mailbox_exec(MAILBOX_CH_ARM2VC, msg)) {
        uart_puts("arm memory:\n");
        uart_puts("size in bytes:");
        uart_hex(msg[6]);
        uart_puts("\n");
        uart_puts("base address:");
        uart_hex(msg[5]);
        uart_puts("\n");
    } else {
        uart_puts("Failed\n");
    }
}
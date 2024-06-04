#include "exception.h"
#include "initramfs.h"
#include "mailbox.h"
#include "malloc.h"
#include "mm.h"
#include "shell.h"
#include "string.h"
#include "timer.h"
#include "uart.h"

void shell_start()
{
    char *line = NULL;
    while (1) {
        uart_puts("# ");
        getline(&line, MAX_GETLINE_LEN);    // FIXME: too many malloc without free?
        do_cmd(line);
    }
}

// Define Shell Commands
static cmd_t command_funcs[] = {
    cmd_help,
    cmd_hello,
    cmd_reboot,
    get_board_revision,
    get_arm_memory,
    list_initramfs,
    cat_initramfs,
    cmd_execute,
    add_timer,
    cmd_malloc,
    cmd_free,
};
static char* commands[] = {
    "help",
    "hello",
    "reboot",
    "board",
    "arm",
    "ls",
    "cat",
    "exec",
    "setTimeout",
    "malloc",
    "free",
};
static char* command_descriptions[] = {
    "print this help menu",
    "print Hello World!",
    "reboot the device",
    "print board info",
    "print arm memory info",
    "list initramfs",
    "cat a file",
    "execute a program in userspace (EL0)",
    "set timeout",
    "allocate memory",
    "free memory",
};

void do_cmd(char* line)
{
    int size = sizeof(command_funcs) / sizeof(command_funcs[0]);
    char *cmd = strtok(line, ' ');
    for (int i = 0; i < size; i++) {
        if (strcmp(cmd, commands[i]) == 0) {
            command_funcs[i](line);
            return;
        }
    }
    cmd_default(line);
    return;
}

void cmd_help(const char *line)
{
    int size = sizeof(command_descriptions) / sizeof(command_descriptions[0]);
    for (int i = 0; i < size; i++) {
        uart_puts(commands[i]);
        uart_puts("\t: ");
        uart_puts(command_descriptions[i]);
        uart_puts("\n");
    }
}

void cmd_hello(const char *line)
{
    uart_puts("Hello World!\n");
}

void cmd_reboot(const char *line)
{
    uart_puts("Reboot!\n");

    unsigned int r;
    r = *PM_RSTS;
    r &= ~0xFFFFFAAA;
    *PM_RSTS = PM_WDOG_MAGIC | r;
    *PM_WDOG = PM_WDOG_MAGIC | 10;
    *PM_RSTC = PM_WDOG_MAGIC | PM_RSTC_FULLRST;
}

void cmd_default(const char *line)
{
    uart_puts("command not found\n");
}

/**
 * Execute binary executable from initramfs in userspace (EL0).
*/
void cmd_execute(const char *line)
{
    char *filename = NULL;
    uart_puts("Filename: ");
    getline(&filename, 0x20);

    // Find file from initramfs
    cpio_meta_t *f = find_initramfs(filename);
    if (f == NULL) {
        return;
    }

    memcpy(&__userspace_start, f->content, f->filesize);    // strlen_new() 會壞掉！因為開頭是 0

    el1_to_el0();
}

void cmd_malloc(const char *line)
{
    char *s = strtok(NULL, ' ');
    if (!s) {
        uart_puts("Usage: malloc [size]\n");
        return;
    }
    int size = atoi(s);
    malloc(size);
}

void cmd_free(const char *line)
{
    char *s = strtok(NULL, ' ');
    if (!s) {
        uart_puts("Usage: free [addr]\n");
        return;
    }
    long addr = atoi(s);
    free((void *) addr);
}

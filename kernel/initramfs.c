#include "cpio.h"
#include "initramfs.h"
#include "malloc.h"
#include "string.h"
#include "uart.h"

static cpio_meta_t *head = NULL;
void parse_initramfs()
{
    cpio_hdr_t *hdr = (cpio_hdr_t *) 0x8000000;    // qemu addr
    // cpio_hdr_t *hdr = (cpio_hdr_t *) 0x20000000;    // rpi3 addr

    char *c;
    int padding, namesize, filesize;
    cpio_meta_t *cur = init_cpio_meta();
    head = cur;

    while (1) {
        namesize = hex_atoi(hdr->c_namesize, 8);
        filesize = hex_atoi(hdr->c_filesize, 8);
        c = (char *) (hdr+1);

        cur->filename = c;
        padding = PADDING_4(sizeof(cpio_hdr_t) + namesize);
        c = (char *) hdr + padding;

        cur->content = c;
        padding = PADDING_4(filesize);
        c += padding;

        if (!strcmp(cur->filename, "TRAILER!!!")) {
            cur->prev->next = NULL;
            break;
        }

        cur->next = init_cpio_meta();
        cur->next->prev = cur;
        cur = cur->next;
        hdr = (cpio_hdr_t *) c;
    }

    uart_puts("initramfs parse complete^^\n");
}

void list_initramfs()
{
    cpio_meta_t *cur = head;
    while (cur != NULL) {
        char *c = cur->filename;

        while (*c != '\0') {
            uart_send(*c++);
        }
        uart_puts("\n");

        cur = cur->next;
    }
}

void cat_initramfs()
{
    uart_puts("Filename: ");

    // TODO: getline (cat and shell)
    char line[1024];
    char c;
    int idx = 0;
    while ((c = (char) uart_getc()) != '\n') {
        line[idx++] = c;
        uart_send(c);
    }
    line[idx++] = '\0';
    uart_send('\n');

    cpio_meta_t *cur = head;
    while (cur != NULL) {
        if (!strcmp(cur->filename, line)) {
            char *ch = cur->content;
            while (*ch != '\0') {
                uart_send(*ch++);
            }
            uart_puts("\n");
            break;
        }
        cur = cur->next;
    }
}

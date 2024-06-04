#ifndef _DEF_SHELL
#define _DEF_SHELL

#define MAX_GETLINE_LEN 0x20

void shell_start(void);
void do_cmd(const char*);

typedef void (*cmd_t)(const char *line);    // line: Original string
void cmd_help(const char *line);
void cmd_hello(const char *line);
void cmd_reboot(const char *line);
void cmd_default(const char *line);
void cmd_execute(const char *line);
void cmd_malloc(const char *line);
void cmd_free(const char *line);

#endif

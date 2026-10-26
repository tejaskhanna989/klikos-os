/* shell.c — KlikOS command shell.
 *
 * Simple interactive shell: reads a line, splits into words, runs builtins
 * or launches external programs.
 */

#include <klikos/unistd.h>
#include <klikos/stdio.h>
#include <klikos/string.h>

#define LINE_MAX 256
#define ARG_MAX  16

static char linebuf[LINE_MAX];

static int shell_read_line(char *buf, int size) {
    int i = 0;
    while (i < size - 1) {
        char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) return -1;
        if (c == '\n' || c == '\r') {
            buf[i] = '\0';
            return i;
        }
        if (c == '\b' || c == 0x7f) {
            if (i > 0) i--;
            continue;
        }
        buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

static int shell_split(char *line, char *argv[], int maxargs) {
    int argc = 0;
    while (*line) {
        while (*line == ' ' || *line == '\t') line++;
        if (!*line) break;
        if (argc >= maxargs) break;
        argv[argc++] = line;
        while (*line && *line != ' ' && *line != '\t') line++;
        if (*line) { *line = '\0'; line++; }
    }
    argv[argc] = NULL;
    return argc;
}

static int shell_builtin(int argc, char **argv) {
    if (argc == 0) return 0;
    if (strcmp(argv[0], "exit") == 0) {
        puts("[shell] exiting");
        return -1;
    }
    if (strcmp(argv[0], "help") == 0) {
        puts("KlikOS shell builtins: exit, help, echo");
        return 0;
    }
    if (strcmp(argv[0], "echo") == 0) {
        for (int i = 1; i < argc; i++) {
            puts(argv[i]);
        }
        return 0;
    }
    return 1; /* not a builtin */
}

int shell_main(int argc, char **argv) {
    puts("KlikOS shell v0.1");
    puts("Type 'help' for builtins, 'exit' to quit.");

    while (1) {
        putchar('>');
        putchar(' ');
        int len = shell_read_line(linebuf, LINE_MAX);
        if (len <= 0) break;

        char *args[ARG_MAX];
        int argc = shell_split(linebuf, args, ARG_MAX);
        if (argc == 0) continue;

        int r = shell_builtin(argc, args);
        if (r == -1) break;   /* exit */
        if (r == 1) {
            puts("[shell] command not found (external exec not yet wired)");
        }
    }

    return 0;
}

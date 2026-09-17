#include "shell.h"
#include "terminal.h"
#include "kernel.h"
#include "keyboard.h"

#define MAX_INPUT 128
#define MAX_ARGS 16

static void shell_print_prompt(void) {
    terminal_write("myos> ");
}

static void parse_command(char *input, char **argv, int *argc) {
    *argc = 0;
    char *current = input;

    while (*current != '\0') {
        while (*current == ' ' || *current == '\t' || *current == '\r' || *current == '\n') {
            *current++ = '\0';
        }
        if (*current == '\0') {
            break;
        }
        argv[(*argc)++] = current;
        while (*current != '\0' && *current != ' ' && *current != '\t' && *current != '\r' && *current != '\n') {
            current++;
        }
    }
}

static int strcmp(const char *a, const char *b) {
    while (*a == *b && *a != '\0') {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

static void command_help(void) {
    terminal_write_line("Available commands:");
    terminal_write_line("  help       Show this help");
    terminal_write_line("  clear      Clear screen");
    terminal_write_line("  echo       Print text");
    terminal_write_line("  reboot     Reboot system");
    terminal_write_line("  time       Display system uptime placeholder");
    terminal_write_line("  mem        Show memory summary");
    terminal_write_line("  whoami     Show current user");
    terminal_write_line("  uname      Show OS information");
    terminal_write_line("  cls        Alias for clear");
}

static void command_clear(void) {
    terminal_clear();
}

static void command_echo(char **argv, int argc) {
    for (int i = 1; i < argc; ++i) {
        terminal_write(argv[i]);
        if (i + 1 < argc) {
            terminal_write(" ");
        }
    }
    terminal_write_line("");
}

static void command_mem(void) {
    terminal_write_line("Memory map: available via BIOS E820 (bootloader stage)");
    terminal_write_line("Kernel heap: static memory pool (placeholder)");
}

static void command_whoami(void) {
    terminal_write_line("root");
}

static void command_uname(void) {
    terminal_write_line("MyOS 0.1 x86 32-bit kernel");
}

static void command_time(void) {
    terminal_write_line("uptime: 0 seconds");
}

static void command_reboot(void) {
    terminal_write_line("Reboot requested. Performing CPU reset...");
    __asm__ volatile ("cli; ljmp $0xFFFF, $0x0000");
    for (;;) {
        __asm__ volatile("hlt");
    }
}

static void execute_command(char *input) {
    char *argv[MAX_ARGS];
    int argc = 0;
    parse_command(input, argv, &argc);

    if (argc == 0) {
        return;
    }

    if (strcmp(argv[0], "help") == 0) {
        command_help();
    } else if (strcmp(argv[0], "clear") == 0 || strcmp(argv[0], "cls") == 0) {
        command_clear();
    } else if (strcmp(argv[0], "echo") == 0) {
        command_echo(argv, argc);
    } else if (strcmp(argv[0], "mem") == 0) {
        command_mem();
    } else if (strcmp(argv[0], "whoami") == 0) {
        command_whoami();
    } else if (strcmp(argv[0], "uname") == 0) {
        command_uname();
    } else if (strcmp(argv[0], "time") == 0) {
        command_time();
    } else if (strcmp(argv[0], "reboot") == 0) {
        command_reboot();
    } else {
        terminal_write("Unknown command: ");
        terminal_write(argv[0]);
        terminal_write_line("");
    }
}

void shell_start(void) {
    char buffer[MAX_INPUT];
    int index = 0;

    terminal_write_line("MyOS shell ready.");
    shell_print_prompt();

    for (;;) {
        int key = keyboard_read_char();
        if (key == 0) {
            continue;
        }

        if (key == '\n') {
            terminal_put_char('\n');
            buffer[index] = '\0';
            execute_command(buffer);
            index = 0;
            shell_print_prompt();
            continue;
        }

        if (key == '\b') {
            if (index > 0) {
                index--;
                terminal_put_char('\b');
                terminal_put_char(' ');
                terminal_put_char('\b');
            }
            continue;
        }

        if (index < MAX_INPUT - 1) {
            buffer[index++] = (char)key;
            terminal_put_char((char)key);
        }
    }
}

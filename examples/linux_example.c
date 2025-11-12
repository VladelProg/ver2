#include <stdio.h>
#include <signal.h>
#include <string.h>
#include "embedded_cli/cli.h"
#include "platform/linux_io.h"

static Cli cli;
static volatile bool running = true;

void signal_handler(int sig) {
    running = false;
    printf("\nReceived signal %d, shutting down...\n\r", sig);
}

int main() {
    printf("Starting Embedded CLI Linux Example...\n\r");
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Инициализация платформозависимого IO
    linux_io_init();
    printf("Linux IO initialized\n\r");
    
    // ВАЖНО: Обнуляем структуру cli перед использованием
    memset(&cli, 0, sizeof(Cli));
    
    // Настройка CLI функций
    cli.write_char = linux_write_char;
    cli.read_char = linux_read_char;
    cli.data_available = linux_data_available;
    
    printf("CLI functions set\n\r");
    
    // Инициализация CLI
    cli_init(&cli);
    printf("CLI initialized\n\r");
    
    printf("Embedded CLI Linux Example Ready\n\r");
    printf("Type 'help' for available commands\n\r");
    
    while (running) {
        cli_process(&cli);
        linux_background_tasks_process();
    }
    
    printf("Goodbye!\n\r");
    return 0;
}

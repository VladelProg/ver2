#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include "embedded_cli/cli.h"

static struct termios old_termios;

static void reset_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

void linux_io_init(void) {
    struct termios new_termios;
    
    // Сохраняем текущие настройки терминала
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    
    // Устанавливаем raw mode
    cfmakeraw(&new_termios);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    
    // Устанавливаем non-blocking ввод
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    
    // Регистрируем функцию восстановления
    atexit(reset_terminal);
}

void linux_write_char(char c) {
    putchar(c);
    fflush(stdout);
}

char linux_read_char(void) {
    return getchar();
}

bool linux_data_available(void) {
    struct timeval tv;
    fd_set fds;
    
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) == 1;
}

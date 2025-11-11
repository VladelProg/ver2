#include <unistd.h>
#include <time.h>
#include "embedded_cli/cli.h"

void linux_background_tasks_init(void) {
    // Инициализация фоновых задач для Linux
}

void linux_background_tasks_process(void) {
    // Обработка фоновых задач
    usleep(1000); // Небольшая задержка для уменьшения нагрузки на CPU
}

uint32_t linux_get_tick(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

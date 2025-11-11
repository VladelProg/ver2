#ifndef LINUX_IO_H
#define LINUX_IO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void linux_io_init(void);
void linux_write_char(char c);
char linux_read_char(void);
bool linux_data_available(void);
void linux_background_tasks_process(void);

#ifdef __cplusplus
}
#endif

#endif // LINUX_IO_H

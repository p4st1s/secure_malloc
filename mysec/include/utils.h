#ifndef _UTILS_H
#define _UTILS_H

#include <stdarg.h>
#include <stdint.h>
#define DEACTIVATE_LOGGING 100

struct tm *get_current_time(void);

// void log_general(const int fd, const char *log_name, const char *format, ...);
void my_log(const char *fmt, ...);
const char *get_log_color(const char *log_level);
int create_log_file(const char *filename);
void init_logging(void);
void close_logging(void);
uint32_t get_random_canary();
#endif
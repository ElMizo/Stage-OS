#include "log.h"
#include <stdio.h>
#include <stdarg.h>

#define LOG_LEVEL_ERR 3

void printk(int level, const char *fmt, ...) {
    va_list args;
    char buffer[1024];
    const char *level_str;

    switch (level) {
        case LOG_LEVEL_ERR: level_str = "ERR"; break;
        default:            level_str = "UNKNOWN"; break;
    }

    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);

    printf("[%s] %s\n", level_str, buffer);
}

void log_error(const char *message) {
    printk(LOG_LEVEL_ERR, "%s", message);
}

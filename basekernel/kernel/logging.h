#ifndef LOGGING_H
#define LOGGING_H

#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_WARN  3
#define LOG_LEVEL_ERROR 4

#define LOG_LEVEL LOG_LEVEL_DEBUG

#define LOG(level, fmt, ...) \
    do { \
        if (level >= LOG_LEVEL) { \
            printf("[LOG %d] " fmt "\n", level, ##__VA_ARGS__); \
        } \
    } while(0)

#endif // LOGGING_H

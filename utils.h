#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <time.h>

#define LOG(fmt, ...)                                                   \
    do {                                                                \
        time_t current_time;                                            \
        struct tm* time_info;                                           \
        char time_string[50];                                           \
        time(&current_time);                                            \
        time_info = localtime(&current_time);                           \
        strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", \
                 time_info);                                            \
        printf("[%s] " fmt, time_string, ##__VA_ARGS__);                \
    } while (0)

enum {
    RET_CODE_SUCCESS = 0,
    RET_CODE_ERROR = 255,
};

void eprintf(const char* fmt, ...);
void unreachable(const char* errmsg);

#endif  // UTILS_H
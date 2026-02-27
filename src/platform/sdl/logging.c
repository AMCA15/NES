#include "../logging.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


void logging_output(LogLevel level, const char* format, va_list args) {
    const char* level_str = NULL;
        
    switch (level) {
        case INFO:
            level_str = "INFO  > ";
            break;
        case DEBUG:
            level_str = "DEBUG > ";
            break;
        case TRACE:
            level_str = "TRACE > ";
            break;
        case ERROR:
            level_str = "ERROR > ";
            break;
        case WARN:
            level_str = "WARN  > ";
            break;
        default:
            level_str = "LOG   > ";
    }
    printf("%s", level_str);
    vprintf(format, args);
    printf("\n");
    fflush(stdout);
}

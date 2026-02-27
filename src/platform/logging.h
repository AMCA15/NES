#pragma once

#include <stdint.h>
#include <stdarg.h>

typedef enum {
    TRACE,    
    DEBUG,    
    INFO,     
    WARN,     
    ERROR,    
} LogLevel;

void logging_output(LogLevel level, const char* format, va_list args);


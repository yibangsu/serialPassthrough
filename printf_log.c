

#include <stdarg.h>
#include <stdio.h>
#include "printf_log.h"

void printfLog(char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stdout, format, ap);
    va_end(ap);
}
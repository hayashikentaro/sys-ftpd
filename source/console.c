// This file is under the terms of the unlicense (https://github.com/DavidBuchanan314/ftpd/blob/master/LICENSE)

#define ENABLE_LOGGING 1
#include "console.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* this is a lot easier when you have a real console */

int should_log = 0;

void console_init(void)
{
}

void console_set_status(const char* fmt, ...)
{
}

// Logging must never replace stdout/stderr or alter the caller's errno.
static void write_log(const char* fmt, va_list ap)
{
    int saved_errno = errno;
    if (should_log)
    {
        FILE* fp = fopen("/config/sys-ftpd/logs/ftpd.log", "a");
        if (fp)
        {
            vfprintf(fp, fmt, ap);
            fclose(fp);
        }
    }
    errno = saved_errno;
}

void console_print(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    write_log(fmt, ap);
    va_end(ap);
}

void debug_print(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    write_log(fmt, ap);
    va_end(ap);
}

void console_render(void)
{
}

#include "diagnostics.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#ifdef __SWITCH__
#include <switch.h>
#endif

#ifndef DIAGNOSTICS_DIRECTORY
#define DIAGNOSTICS_DIRECTORY "/config/sys-ftpd/logs"
#endif
#ifndef DIAGNOSTICS_MARKER
#define DIAGNOSTICS_MARKER "/config/sys-ftpd/diagnostics_enabled"
#endif
#ifndef DIAGNOSTICS_LIMIT
#define DIAGNOSTICS_LIMIT (2 * 1024 * 1024)
#endif
#define LOG_PATH DIAGNOSTICS_DIRECTORY "/diagnostic.log"
#define OLD_PATH DIAGNOSTICS_DIRECTORY "/diagnostic.previous.log"

static int enabled;
static unsigned long sequence;
static long log_size;

int diagnostics_enabled(void)
{
    return enabled;
}

void diagnostics_init(void)
{
    int saved_errno = errno;
    struct stat st;
    enabled = 0;
    if (stat(DIAGNOSTICS_MARKER, &st) == 0 &&
        (mkdir(DIAGNOSTICS_DIRECTORY, 0700) == 0 || errno == EEXIST))
    {
        // Preserve the previous boot's evidence, including an unclean shutdown.
        if (stat(LOG_PATH, &st) == 0 && rename(LOG_PATH, OLD_PATH) != 0)
        {
            errno = saved_errno;
            return;
        }
        enabled = 1;
        log_size = 0;
        sequence = 0;
        diagnostics_log("startup build=" __DATE__ " " __TIME__);
    }
    errno = saved_errno;
}

void diagnostics_log(const char* fmt, ...)
{
    int saved_errno = errno;
    if (enabled)
    {
        // Bound storage use to two logs. Rotation failure disables diagnostics.
        if (log_size >= DIAGNOSTICS_LIMIT)
        {
            if (rename(LOG_PATH, OLD_PATH) != 0)
            {
                enabled = 0;
                errno = saved_errno;
                return;
            }
            log_size = 0;
        }
        FILE* fp = fopen(LOG_PATH, "a");
        if (fp)
        {
#ifdef __SWITCH__
            unsigned long long elapsed_ms = armTicksToNs(svcGetSystemTick()) / 1000000;
#else
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC, &ts);
            unsigned long long elapsed_ms = (unsigned long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
            fprintf(fp, "seq=%lu ms=%llu ", ++sequence, elapsed_ms);
            va_list ap;
            va_start(ap, fmt);
            vfprintf(fp, fmt, ap);
            va_end(ap);
            fputc('\n', fp);
            long position = ftell(fp);
            if (position >= 0)
                log_size = position;
            // Close each event so logs survive process exit and are readable by FTP.
            fclose(fp);
        }
    }
    errno = saved_errno;
}

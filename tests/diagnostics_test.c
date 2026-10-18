#include "diagnostics.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
    errno = EBUSY;
    diagnostics_init();
    assert(!diagnostics_enabled() && errno == EBUSY);
    FILE* fp = fopen(DIAGNOSTICS_MARKER, "w");
    assert(fp && fclose(fp) == 0);
    diagnostics_init();
    assert(diagnostics_enabled() && errno == EBUSY);
    diagnostics_log("known errno preservation %d", 451);
    assert(errno == EBUSY);
    // Exercise size rotation and verify a second boot preserves evidence.
    for (int i = 0; i < 30; ++i)
        diagnostics_log("record=%d errno=%d", i, ENOMEM);
    struct stat st;
    assert(stat(DIAGNOSTICS_DIRECTORY "/diagnostic.previous.log", &st) == 0);
    assert(st.st_size <= DIAGNOSTICS_LIMIT + 128);
    diagnostics_init();
    assert(diagnostics_enabled() && errno == EBUSY);
    assert(stat(DIAGNOSTICS_DIRECTORY "/diagnostic.previous.log", &st) == 0);
    // A missing/unwritable log directory must not crash or alter caller errno.
    assert(unlink(DIAGNOSTICS_DIRECTORY "/diagnostic.log") == 0);
    assert(unlink(DIAGNOSTICS_DIRECTORY "/diagnostic.previous.log") == 0);
    assert(rmdir(DIAGNOSTICS_DIRECTORY) == 0);
    errno = EMFILE;
    diagnostics_log("unavailable destination");
    assert(errno == EMFILE);
    puts("PASS disabled marker, errno preservation, rotation, boot preservation, unavailable log path");
    return 0;
}

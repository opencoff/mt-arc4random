#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void
error(int doexit, int err, const char* fmt, ...)
{
    va_list ap;

    fflush(stdout);
    fflush(stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (err > 0)
        fprintf(stderr, "\n  %s (Errno %d)\n", strerror(err), err);

    if (doexit) {
        fflush(stderr);
        exit(1);
    }
}

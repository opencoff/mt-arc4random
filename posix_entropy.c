/* vim: expandtab:tw=68:ts=4:sw=4:
 *
 * posix_entropy.c - Entropy interface for POSIX systems.
 *
 * Copyright (c) 2015 Sudhi Herle <sw at herle.net>
 *
 * Licensing Terms: GPLv2 
 *
 * If you need a commercial license for this work, please contact
 * the author.
 *
 * This software does not come with any express or implied
 * warranty; it is provided "as is". No claim  is made to its
 * suitability for any purpose.
 */


#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include "error.h"

static int
randopen(const char* name)
{
    int fd = open(name, O_RDONLY);
    if (fd < 0)
        error(1, errno, "Cannot open system random number dev %s", name);


    return fd;
}


void*
sys_entropy(void* buf, size_t n)
{
    uint8_t* b = (uint8_t*)buf;
    int fd     = randopen("/dev/random");

    while (n > 0)
    {
        ssize_t m = (read)(fd, b, n);

        if (m < 0) {
            if (errno == EINTR) continue;
            error(1, errno, "Fatal read error while reading rand dev");
        }
        b += m;
        n -= m;
    }

    close(fd);
    return buf;
}



/* EOF */

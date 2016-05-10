/*
 * Simple test harness and benchmark for MT Arc4Random
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "arc4random.h"
#include "cputime.h"


extern void error(int doexit, int err, const char* fmt, ...);


/*
 * Generate 'siz' byte RNG in a tight loop and provide averages.
 */
static void
bench(int fd, size_t siz, size_t niter)
{
    size_t j;
    uint8_t *buf = malloc(siz);
    uint64_t s0, s1, s2;
    uint64_t ta = 0;    // cumulative time for arc4rand
    uint64_t ts = 0;    // cumulative time for system rand
    ssize_t m;

    for (j = 0; j < niter; ++j) {
        s0 = sys_cpu_timestamp();
        arc4random_buf(buf, siz);
        s1 = sys_cpu_timestamp();
        m  =  read(fd, buf, siz);
        s2 = sys_cpu_timestamp();

        if (m < 0) error(1, errno, "Partial read from /dev/urandom; exp %d, saw %d", siz, m);

        ta += s1 - s0;
        ts += s2 - s1;
    }

#define _d(x)   ((double)(x))
    double aa = _d(ta) / _d(niter);     // average of n runs for arc4random
    double as = _d(ts) / _d(niter);     // average of n runs for sysrand
    double sa = aa / _d(siz);           // cycles/byte for arc4random
    double ss = as / _d(siz);           // cycles/byte for sysrand

    printf("%6lu, %9.4f %9.4f\n", siz, sa, ss);

    free(buf);
}



#define NITER       8192

int
main(int argc, const char** argv)
{

    if (argc > 1) {
        int i;

        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) error(1, errno, "Can't open dev/urandom");


        printf("size,      arc4rand cy/byte,   sysrand cy/byte\n");
        for (i = 1; i < argc; ++i) {
            int z = atoi(argv[i]);
            if (z <= 0) continue;
            bench(fd, z, NITER);
        }


        close(fd);
    }

    return 0;
}


/* EOF */

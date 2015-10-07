/*
 * Copyright (c) 1996, David Mazieres <dm@uun.org>
 * Copyright (c) 2008, Damien Miller <djm@openbsd.org>
 * Copyright (c) 2013, Markus Friedl <markus@openbsd.org>
 * Copyright (c) 2014, Theo de Raadt <deraadt@openbsd.org>
 * Copyright (c) 2015, Sudhi Herle   <sudhi@herle.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/*
 * ChaCha based random number generator from OpenBSD.
 *
 * Made fully portable and thread-safe by Sudhi Herle.
 */

#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <assert.h>
#include <pthread.h>


#define ARC4R_KEYSZ     32
#define ARC4R_IVSZ      8
#define ARC4R_BLOCKSZ   64
#define ARC4R_RSBUFSZ   (16*ARC4R_BLOCKSZ)

typedef struct
{
    uint32_t input[16]; /* could be compressed */
} zz_chacha_ctx;

struct zzrand_state
{
    size_t          rs_have;    /* valid bytes at end of rs_buf */
    size_t          rs_count;   /* bytes till reseed */
    pid_t           rs_pid;     /* My PID */
    zz_chacha_ctx   rs_chacha;  /* chacha context for random keystream */
    u_char          rs_buf[ARC4R_RSBUFSZ];  /* keystream blocks */
};
typedef struct zzrand_state zzrand_state;



#define KEYSTREAM_ONLY
#include "chacha_private.h"

#define minimum(a, b) ((a) < (b) ? (a) : (b))




static inline void
_rs_init(zzrand_state* st, u8 *buf, size_t n)
{
    assert(n >= (ARC4R_KEYSZ + ARC4R_IVSZ));

    chacha_keysetup(&st->rs_chacha, buf, ARC4R_KEYSZ * 8, 0);
    chacha_ivsetup(&st->rs_chacha,  buf + ARC4R_KEYSZ);
    memset(buf, 0x55, ARC4R_KEYSZ + ARC4R_IVSZ);
}



static inline void
_rs_rekey(zzrand_state* st, u8 *dat, size_t datlen)
{
    /* fill rs_buf with the keystream */
    chacha_encrypt_bytes(&st->rs_chacha, st->rs_buf, st->rs_buf, sizeof st->rs_buf);

    /* mix in optional user provided data */
    if (dat) {
        size_t i, m;

        m = minimum(datlen, ARC4R_KEYSZ + ARC4R_IVSZ);
        for (i = 0; i < m; i++)
            st->rs_buf[i] ^= dat[i];

        memset(dat, 0, datlen);
    }

    /* immediately reinit for backtracking resistance */
    _rs_init(st, st->rs_buf, ARC4R_KEYSZ + ARC4R_IVSZ);
    memset(st->rs_buf, 0, ARC4R_KEYSZ + ARC4R_IVSZ);
    st->rs_have = (sizeof st->rs_buf) - ARC4R_KEYSZ - ARC4R_IVSZ;
}


static void
_rs_stir(zzrand_state* st)
{
    u8 rnd[ARC4R_KEYSZ + ARC4R_IVSZ];

    /*
     * The platform should provide this API to get entropy.
     */
    extern void* sys_entropy(void*, size_t);

    sys_entropy(rnd, sizeof rnd);

    _rs_rekey(st, rnd, sizeof(rnd));

    /* invalidate rs_buf */
    st->rs_have = 0;
    memset(st->rs_buf, 0, sizeof st->rs_buf);

    _rs_rekey(st, 0, 0);

    st->rs_count = 1600000;
}


static inline void
_rs_stir_if_needed(zzrand_state* st, size_t len)
{
    if (st->rs_count <= len)
        _rs_stir(st);
    else
        st->rs_count -= len;
}


static inline void
_rs_random_buf(zzrand_state* rs, void *_buf, size_t n)
{
    u8 *buf = (u8 *)_buf;
    u8 *keystream;
    size_t m;

    _rs_stir_if_needed(rs, n);
    while (n > 0) {
        if (rs->rs_have > 0) {
            m = minimum(n, rs->rs_have);
            keystream = rs->rs_buf + sizeof(rs->rs_buf) - rs->rs_have;
            memcpy(buf, keystream, m);
            memset(keystream, 0, m);
            buf += m;
            n   -= m;
            rs->rs_have -= m;
        } else 
            _rs_rekey(rs, NULL, 0);
    }
}

static inline uint32_t
_rs_random_u32(zzrand_state* rs)
{
    u8 *keystream;
    uint32_t val;

    _rs_stir_if_needed(rs, sizeof(val));
    if (rs->rs_have < sizeof(val))
        _rs_rekey(rs, NULL, 0);
    keystream = rs->rs_buf + sizeof(rs->rs_buf) - rs->rs_have;
    memcpy(&val, keystream, sizeof(val));
    memset(keystream, 0, sizeof(val));
    rs->rs_have -= sizeof(val);

    return val;
}



/*
 * Multi-threaded support using pthread API.
 *
 * XXX It may be better to use function-local static variable
 * annotated with the gcc "__thread" attribute. This avoids a
 * potentially failure prone run-time memory allocation.
 */
static pthread_key_t  Rkey;
static pthread_once_t Ronce   = PTHREAD_ONCE_INIT;
static uint32_t       Rforked = 0;

/*
 * Fork handler to reset my context
 */
static void
atfork()
{
    // XXX GCC Intrinsyc
    __sync_fetch_and_add(&Rforked, 1);
}

/*
 * Run once and only once by pthread lib. We use the opportunity to
 * create the thread-specific key.
 */
static void
zcreate()
{
    pthread_key_create(&Rkey, 0);
    pthread_atfork(0, 0, atfork);
}


/*
 * Get the per-thread rand state. Initialize if needed.
 */
static zzrand_state*
zinit()
{
    pthread_once(&Ronce, zcreate);

    volatile pthread_key_t* k = &Rkey;
    zzrand_state * z = (zzrand_state *)pthread_getspecific(*k);
    if (!z) {
        z = (zzrand_state*)calloc(sizeof *z, 1);
        assert(z);

        _rs_stir(z);
        z->rs_pid = getpid();

        pthread_setspecific(*k, z);
    }

    /* Detect if a fork has happened */
    if (Rforked > 0 || getpid() != z->rs_pid) {
        __sync_fetch_and_sub(&Rforked, 1);
        z->rs_pid = getpid();
        _rs_stir(z);
    }

    return z;
}


/*
 * Public API
 */


uint32_t
arc4random()
{
    zzrand_state* z = zinit();

    return _rs_random_u32(z);
}


void
arc4random_buf(void* b, size_t n)
{
    zzrand_state* z = zinit();

    _rs_random_buf(z, b, n);
}




/*
 * Calculate a uniformly distributed random number less than upper_bound
 * avoiding "modulo bias".
 *
 * Uniformity is achieved by generating new random numbers until the one
 * returned is outside the range [0, 2**32 % upper_bound).  This
 * guarantees the selected random number will be inside
 * [2**32 % upper_bound, 2**32) which maps back to [0, upper_bound)
 * after reduction modulo upper_bound.
 */
uint32_t
arc4random_uniform(uint32_t upper_bound)
{
    zzrand_state* z = zinit();
    uint32_t r, min;

    if (upper_bound < 2)
        return 0;

    /* 2**32 % x == (2**32 - x) % x */
    min = -upper_bound % upper_bound;

    /*
     * This could theoretically loop forever but each retry has
     * p > 0.5 (worst case, usually far better) of selecting a
     * number inside the range we need, so it should rarely need
     * to re-roll.
     */
    for (;;) {
        r = _rs_random_u32(z);
        if (r >= min)
            break;
    }

    return r % upper_bound;
}

/* EOF */

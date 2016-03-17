/* vim: expandtab:tw=68:ts=4:sw=4:
 *
 * randuuid.c - UUID Generator based on Arc4Random
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
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#ifdef __OpenBSD__

#define ARC4RANDOM          mt_arc4random
#define ARC4RANDOM_UNIFORM  mt_arc4random_uniform
#define ARC4RANDOM_BUF      mt_arc4random_buf

#else

#define ARC4RANDOM          arc4random
#define ARC4RANDOM_UNIFORM  arc4random_uniform
#define ARC4RANDOM_BUF      arc4random_buf

#endif /* __OpenBSD__ */

extern void ARC4RANDOM_BUF(void* b, size_t n);


/*
 * Generate a random UUID
 *
 * n should be at least  16 bytes long.
 */
void
randuuid(uint8_t* b, size_t n)
{
    if (n > 16) n = 16;

    ARC4RANDOM_BUF(b, n);
    if (n > 8) {

        /*
         * Fixup version etc.
         */
        b[8] &= 0xbf;
        b[8] |= 0x80;

        b[6] &= 0x4f;
        b[6] |= 0x40;
    }
}
/* EOF */

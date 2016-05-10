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

#include "arc4random.h"


/*
 * Generate a random UUID
 *
 * n should be at least  16 bytes long.
 */
void
randuuid(uint8_t* b, size_t n)
{
    if (n > 16) n = 16;

    arc4random_buf(b, n);
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

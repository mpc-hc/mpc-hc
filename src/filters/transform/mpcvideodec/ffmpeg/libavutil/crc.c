/*
 * copyright (c) 2006 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "common.h"
#include "crc.h"

AVCRC av_crcEDB88320[257];
AVCRC av_crc04C11DB7[257];
AVCRC av_crc8005    [257];
AVCRC av_crc07      [257];

/**
 * Inits a crc table.
 * @param ctx must be an array of sizeof(AVCRC)*257 or sizeof(AVCRC)*1024
 * @param cts_size size of ctx in bytes
 * @param le if 1, lowest bit represents coefficient for highest exponent
 *           of corresponding polynomial (both for poly and actual CRC).
 *           If 0, you must swap the crc parameter and the result of av_crc
 *           if you need the standard representation (can be simplified in
 *           most cases to e.g. bswap16):
 *           bswap_32(crc << (32-bits))
 * @param bits number of bits for the CRC
 * @param poly generator polynomial without the x**bits coefficient, in the
 *             representation as specified by le
 * @return <0 on failure
 */
int av_crc_init(AVCRC *ctx, int le, int bits, uint32_t poly, int ctx_size){
    int i, j;
    uint32_t c;

    if (bits < 8 || bits > 32 || poly >= (1LL<<bits))
        return -1;
    if (ctx_size != sizeof(AVCRC)*257 && ctx_size != sizeof(AVCRC)*1024)
        return -1;

    for (i = 0; i < 256; i++) {
        if (le) {
            for (c = i, j = 0; j < 8; j++)
                c = (c>>1)^(poly & (-(c&1)));
            ctx[i] = c;
        } else {
            for (c = i << 24, j = 0; j < 8; j++)
                c = (c<<1) ^ ((poly<<(32-bits)) & (((int32_t)c)>>31) );
            ctx[i] = bswap_32(c);
        }
    }
    ctx[256]=1;
#ifndef CONFIG_SMALL
    if(ctx_size >= sizeof(AVCRC)*1024)
        for (i = 0; i < 256; i++)
            for(j=0; j<3; j++)
                ctx[256*(j+1) + i]= (ctx[256*j + i]>>8) ^ ctx[ ctx[256*j + i]&0xFF ];
#endif

    return 0;
}

/**
 * Calculate the CRC of a block
 * @param crc CRC of previous blocks if any or initial value for CRC.
 * @return CRC updated with the data from the given block
 *
 * @see av_crc_init() "le" parameter
 */
uint32_t av_crc(const AVCRC *ctx, uint32_t crc, const uint8_t *buffer, size_t length){
    const uint8_t *end= buffer+length;

#ifndef CONFIG_SMALL
    if(!ctx[256])
        while(buffer<end-3){
            crc ^= le2me_32(*(uint32_t*)buffer); buffer+=4;
            crc =  ctx[3*256 + ( crc     &0xFF)]
                  ^ctx[2*256 + ((crc>>8 )&0xFF)]
                  ^ctx[1*256 + ((crc>>16)&0xFF)]
                  ^ctx[0*256 + ((crc>>24)     )];
        }
#endif
    while(buffer<end)
        crc = ctx[((uint8_t)crc) ^ *buffer++] ^ (crc >> 8);

    return crc;
}

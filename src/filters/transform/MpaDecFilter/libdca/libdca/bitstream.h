/*
 * bitstream.h
 * Copyright (C) 2004 Gildas Bazin <gbazin@videolan.org>
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of libdca, a free DTS Coherent Acoustics stream decoder.
 * See http://www.videolan.org/developers/libdca.html for updates.
 *
 * libdca is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * libdca is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef WORDS_BIGENDIAN

#   define swab32(x) (x)

#else

#   define swab32(x)\
((((uint8_t*)&x)[0] << 24) | (((uint8_t*)&x)[1] << 16) |  \
 (((uint8_t*)&x)[2] << 8)  | (((uint8_t*)&x)[3]))

#endif

#ifdef WORDS_BIGENDIAN

#   define swable32(x)\
((((uint8_t*)&x)[0] << 16) | (((uint8_t*)&x)[1] << 24) |  \
 (((uint8_t*)&x)[2])  | (((uint8_t*)&x)[3] << 8))

#else

#   define swable32(x)\
((((uint32_t)x) >> 16) | (((uint32_t)x) << 16))

#endif

void dca_bitstream_init (dca_state_t * state, uint8_t * buf, int word_mode,
                         int endian_mode);
uint32_t dca_bitstream_get_bh (dca_state_t * state, uint32_t num_bits);

static inline uint32_t bitstream_get (dca_state_t * state, uint32_t num_bits)
{
    uint32_t result;

    if (num_bits < state->bits_left) {
        result = (state->current_word << (32 - state->bits_left))
				      >> (32 - num_bits);

        state->bits_left -= num_bits;
        return result;
    }

    return dca_bitstream_get_bh (state, num_bits);
}

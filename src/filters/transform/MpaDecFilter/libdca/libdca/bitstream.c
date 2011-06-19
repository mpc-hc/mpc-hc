/*
 * bitstream.c
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

#include "config.h"

#include <stdio.h>
#include <inttypes.h>

#include "dca.h"
#include "dca_internal.h"
#include "bitstream.h"

#define BUFFER_SIZE 4096

void dca_bitstream_init (dca_state_t * state, uint8_t * buf, int word_mode,
                         int bigendian_mode)
{
    intptr_t align;

    align = (uintptr_t)buf & 3;
    state->buffer_start = (uint32_t *) ((uintptr_t)buf - align);
    state->bits_left = 0;
    state->current_word = 0;
    state->word_mode = word_mode;
    state->bigendian_mode = bigendian_mode;
    bitstream_get (state, align * 8);
}

static inline void bitstream_fill_current (dca_state_t * state)
{
    uint32_t tmp;

    tmp = *(state->buffer_start++);

    if (state->bigendian_mode)
        state->current_word = swab32 (tmp);
    else
        state->current_word = swable32 (tmp);

    if (!state->word_mode)
    {
        state->current_word = (state->current_word & 0x00003FFF) |
            ((state->current_word & 0x3FFF0000 ) >> 2);
    }
}

/*
 * The fast paths for _get is in the
 * bitstream.h header file so it can be inlined.
 *
 * The "bottom half" of this routine is suffixed _bh
 *
 * -ah
 */

uint32_t dca_bitstream_get_bh (dca_state_t * state, uint32_t num_bits)
{
    uint32_t result = 0;

    if (state->bits_left)
    {
	num_bits -= state->bits_left;
        result = ((state->current_word << (32 - state->bits_left)) >>
		  (32 - state->bits_left));
    }

    if ( !state->word_mode && num_bits > 28 ) {
        bitstream_fill_current (state);
	result = (result << 28) | state->current_word;
	num_bits -= 28;
    }

    bitstream_fill_current (state);

    if ( state->word_mode )
    {
        if (num_bits != 0)
	    result = (result << num_bits) |
	             (state->current_word >> (32 - num_bits));

	state->bits_left = 32 - num_bits;
    }
    else
    {
        if (num_bits != 0)
	    result = (result << num_bits) |
	             (state->current_word >> (28 - num_bits));

	state->bits_left = 28 - num_bits;
    }

    return result;
}

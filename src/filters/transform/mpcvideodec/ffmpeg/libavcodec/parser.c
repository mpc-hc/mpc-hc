/*
 * Audio and Video frame extraction
 * Copyright (c) 2003 Fabrice Bellard.
 * Copyright (c) 2003 Michael Niedermayer.
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

#include "parser.h"

/*****************************************************/

/**
 * combines the (truncated) bitstream to a complete frame
 * @returns -1 if no complete frame could be created
 */
int ff_combine_frame(ParseContext *pc, int next, const uint8_t **buf, int *buf_size)
{
#if 0
    if(pc->overread){
        printf("overread %d, state:%X next:%d index:%d o_index:%d\n", pc->overread, pc->state, next, pc->index, pc->overread_index);
        printf("%X %X %X %X\n", (*buf)[0], (*buf)[1],(*buf)[2],(*buf)[3]);
    }
#endif

    /* Copy overread bytes from last frame into buffer. */
    for(; pc->overread>0; pc->overread--){
        pc->buffer[pc->index++]= pc->buffer[pc->overread_index++];
    }

    /* flush remaining if EOF */
    if(!*buf_size && next == END_NOT_FOUND){
        next= 0;
    }

    pc->last_index= pc->index;

    /* copy into buffer end return */
    if(next == END_NOT_FOUND){
        pc->buffer= av_fast_realloc(pc->buffer, &pc->buffer_size, (*buf_size) + pc->index + FF_INPUT_BUFFER_PADDING_SIZE);

        memcpy(&pc->buffer[pc->index], *buf, *buf_size);
        pc->index += *buf_size;
        return -1;
    }

    *buf_size=
    pc->overread_index= pc->index + next;

    /* append to buffer */
    if(pc->index){
        pc->buffer= av_fast_realloc(pc->buffer, &pc->buffer_size, next + pc->index + FF_INPUT_BUFFER_PADDING_SIZE);

        memcpy(&pc->buffer[pc->index], *buf, next + FF_INPUT_BUFFER_PADDING_SIZE );
        pc->index = 0;
        *buf= pc->buffer;
    }

    /* store overread bytes */
    for(;next < 0; next++){
        pc->state = (pc->state<<8) | pc->buffer[pc->last_index + next];
        pc->overread++;
    }

#if 0
    if(pc->overread){
        printf("overread %d, state:%X next:%d index:%d o_index:%d\n", pc->overread, pc->state, next, pc->index, pc->overread_index);
        printf("%X %X %X %X\n", (*buf)[0], (*buf)[1],(*buf)[2],(*buf)[3]);
    }
#endif

    return 0;
}

/*************************/

int ff_mpeg4video_split(AVCodecContext *avctx,
                           const uint8_t *buf, int buf_size)
{
    int i;
    uint32_t state= -1;

    for(i=0; i<buf_size; i++){
        state= (state<<8) | buf[i];
        if(state == 0x1B3 || state == 0x1B6)
            return i-3;
    }
    return 0;
}


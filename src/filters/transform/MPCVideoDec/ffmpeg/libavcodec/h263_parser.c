/*
 * H.263 parser
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
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

/**
 * @file libavcodec/h263_parser.c
 * H.263 parser
 */

#include "parser.h"
#include "h263_parser.h"

int ff_h263_find_frame_end(ParseContext *pc, const uint8_t *buf, int buf_size){
    int vop_found, i;
    uint32_t state;

    vop_found= pc->frame_start_found;
    state= pc->state;

    i=0;
    if(!vop_found){
        for(i=0; i<buf_size; i++){
            state= (state<<8) | buf[i];
            if(state>>(32-22) == 0x20){
                i++;
                vop_found=1;
                break;
            }
        }
    }

    if(vop_found){
      for(; i<buf_size; i++){
        state= (state<<8) | buf[i];
        if(state>>(32-22) == 0x20){
            pc->frame_start_found=0;
            pc->state=-1;
            return i-3;
        }
      }
    }
    pc->frame_start_found= vop_found;
    pc->state= state;

    return END_NOT_FOUND;
}

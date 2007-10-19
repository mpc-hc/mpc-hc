/*
 * H.26L/H.264/AVC/JVT/14496-10/... parser
 * Copyright (c) 2003 Michael Niedermayer <michaelni@gmx.at>
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
 *
 */

/**
 * @file h264_parser.c
 * H.264 / AVC / MPEG4 part10 parser.
 * @author Michael Niedermayer <michaelni@gmx.at>
 */

#include "parser.h"
#include "h264_parser.h"

#include <assert.h>


int ff_h264_find_frame_end(H264Context *h, const uint8_t *buf, int buf_size)
{
    int i;
    uint32_t state;
    ParseContext *pc = &(h->s.parse_context);
//printf("first %02X%02X%02X%02X\n", buf[0], buf[1],buf[2],buf[3]);
//    mb_addr= pc->mb_addr - 1;
    state= pc->state;
    if(state>13)
        state= 7;

    for(i=0; i<buf_size; i++){
        if(state==7){
            for(; i<buf_size; i++){
                if(!buf[i]){
                    state=2;
                    break;
                }
            }
        }else if(state<=2){
            if(buf[i]==1)   state^= 5; //2->7, 1->4, 0->5
            else if(buf[i]) state = 7;
            else            state>>=1; //2->1, 1->0, 0->0
        }else if(state<=5){
            int v= buf[i] & 0x1F;
            if(v==7 || v==8 || v==9){
                if(pc->frame_start_found){
                    i++;
found:
                    pc->state=7;
                    pc->frame_start_found= 0;
                    return i-(state&5);
                }
            }else if(v==1 || v==2 || v==5){
                if(pc->frame_start_found){
                    state+=8;
                    continue;
                }else
                    pc->frame_start_found = 1;
            }
            state= 7;
        }else{
            if(buf[i] & 0x80)
                goto found;
            state= 7;
        }
    }
    pc->state= state;
    return END_NOT_FOUND;
}

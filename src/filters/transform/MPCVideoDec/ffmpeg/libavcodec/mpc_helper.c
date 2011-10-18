/*
 * This file is part of Media Player Classic HomeCinema.
 *
 * MPC-HC is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "avcodec.h"
#include "internal.h"
#include "ac3.h"
#include "ac3_parser.h"
#include "ac3dec.h"
#include "ac3dec_data.h"

#ifdef __GNUC__
#define _aligned_malloc  __mingw_aligned_malloc
#define _aligned_realloc __mingw_aligned_realloc
#define _aligned_free    __mingw_aligned_free
#endif

int FFGetChannelMap(struct AVCodecContext * avctx)
{
    switch (avctx->codec_id)
    {
    case CODEC_ID_EAC3:
    case CODEC_ID_AC3:
        {
            AC3DecodeContext *s = avctx->priv_data;

            // Mapping index for s_scmap_ac3
            switch (s->channel_mode)
            {
            case AC3_CHMODE_DUALMONO:   return 0;
            case AC3_CHMODE_MONO:       return 1;
            case AC3_CHMODE_STEREO:     return 2;
            case AC3_CHMODE_3F:         return 3;
            case AC3_CHMODE_2F1R:       return 4;
            case AC3_CHMODE_3F1R:       return 5;
            case AC3_CHMODE_2F2R:       return 6;
            case AC3_CHMODE_3F2R:       return (s->lfe_on ? 8 : 7);
            }
        }
        break;
    case CODEC_ID_MLP:
        {
            // Mapping index for s_scmap_lpcm
            if (avctx->channels <= 8)
                return avctx->channels-1;
            else
                return -1;
        }
    default:
        return 2;
    }
    return -1;
}


void* FF_aligned_malloc(size_t size,size_t alignment)
{
    return _aligned_malloc(size,alignment);
}

void FF_aligned_free(void* mem_ptr)
{
    if (mem_ptr)
        _aligned_free(mem_ptr);
}

void* FF_aligned_realloc(void *ptr,size_t size,size_t alignment)
{
    if (!ptr)
        return FF_aligned_malloc(size,alignment);
    else
        if (size == 0)
        {
            FF_aligned_free(ptr);
            return NULL;
        }
        else
            return _aligned_realloc(ptr,size,alignment);
}

/*
 * (C) 2009-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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

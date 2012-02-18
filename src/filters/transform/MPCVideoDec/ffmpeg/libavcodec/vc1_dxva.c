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

int av_vc1_decode_frame(AVCodecContext *avctx, const uint8_t *buf, int buf_size, int *nFrameSize)
{
    int n_slices = 0, i;
    VC1Context *v = avctx->priv_data;
    MpegEncContext *s = &v->s;
    uint8_t *buf2 = NULL;
    const uint8_t *buf_start = buf, *buf_start_second_field = NULL;
    struct {
        uint8_t *buf;
        GetBitContext gb;
        int mby_start;
    } *slices = NULL, *tmp;

    v->second_field = 0;
    *nFrameSize = 0;	

    //for advanced profile we may need to parse and unescape data
    if (avctx->codec_id == CODEC_ID_VC1 || avctx->codec_id == CODEC_ID_VC1IMAGE) {
        int buf_size2 = 0;
        buf2 = av_mallocz(buf_size + FF_INPUT_BUFFER_PADDING_SIZE);

        if (IS_MARKER(AV_RB32(buf))) { /* frame starts with marker and needs to be parsed */
            const uint8_t *start, *end, *next;
            int size;

            next = buf;
            for (start = buf, end = buf + buf_size; next < end; start = next) {
                next = find_next_marker(start + 4, end);
                size = next - start - 4;
                if (size <= 0) continue;
                switch (AV_RB32(start)) {
                case VC1_CODE_FRAME:
                    buf_start = start;
                    buf_size2 = vc1_unescape_buffer(start + 4, size, buf2);
                    break;
                case VC1_CODE_FIELD: {
                    int buf_size3;
                    buf_start_second_field = start;
                    slices = av_realloc(slices, sizeof(*slices) * (n_slices+1));
                    if (!slices)
                        goto err;
                    slices[n_slices].buf = av_mallocz(buf_size + FF_INPUT_BUFFER_PADDING_SIZE);
                    if (!slices[n_slices].buf)
                        goto err;
                    buf_size3 = vc1_unescape_buffer(start + 4, size,
                                                    slices[n_slices].buf);
                    init_get_bits(&slices[n_slices].gb, slices[n_slices].buf,
                                  buf_size3 << 3);
                    /* assuming that the field marker is at the exact middle,
                       hope it's correct */
                    slices[n_slices].mby_start = s->mb_height >> 1;
                    n_slices++;
                    break;
                }
                case VC1_CODE_ENTRYPOINT: /* it should be before frame data */
                    buf_size2 = vc1_unescape_buffer(start + 4, size, buf2);
                    init_get_bits(&s->gb, buf2, buf_size2 * 8);
                    vc1_decode_entry_point(avctx, v, &s->gb);
                    break;
                case VC1_CODE_SLICE: {
                    int buf_size3;
                    slices = av_realloc(slices, sizeof(*slices) * (n_slices+1));
                    if (!slices)
                        goto err;
                    slices[n_slices].buf = av_mallocz(buf_size + FF_INPUT_BUFFER_PADDING_SIZE);
                    if (!slices[n_slices].buf)
                        goto err;
                    buf_size3 = vc1_unescape_buffer(start + 4, size,
                                                    slices[n_slices].buf);
                    init_get_bits(&slices[n_slices].gb, slices[n_slices].buf,
                                  buf_size3 << 3);
                    slices[n_slices].mby_start = get_bits(&slices[n_slices].gb, 9);
                    n_slices++;
                    break;
                }
                }
            }
        } else if (v->interlace && ((buf[0] & 0xC0) == 0xC0)) { /* WVC1 interlaced stores both fields divided by marker */
            const uint8_t *divider;
            int buf_size3;

            divider = find_next_marker(buf, buf + buf_size);
            if ((divider == (buf + buf_size)) || AV_RB32(divider) != VC1_CODE_FIELD) {
                av_log(avctx, AV_LOG_ERROR, "Error in WVC1 interlaced frame\n");
                goto err;
            } else { // found field marker, unescape second field
            	buf_start_second_field = divider;
                tmp = av_realloc(slices, sizeof(*slices) * (n_slices+1));
                if (!tmp)
                    goto err;
                slices = tmp;
                slices[n_slices].buf = av_mallocz(buf_size + FF_INPUT_BUFFER_PADDING_SIZE);
                if (!slices[n_slices].buf)
                    goto err;
                buf_size3 = vc1_unescape_buffer(divider + 4, buf + buf_size - divider - 4, slices[n_slices].buf);
                init_get_bits(&slices[n_slices].gb, slices[n_slices].buf,
                              buf_size3 << 3);
                slices[n_slices].mby_start = s->mb_height >> 1;
                n_slices++;
            }
            buf_size2 = vc1_unescape_buffer(buf, divider - buf, buf2);
        } else {
            buf_size2 = vc1_unescape_buffer(buf, buf_size, buf2);
        }
        init_get_bits(&s->gb, buf2, buf_size2*8);
    } else
        init_get_bits(&s->gb, buf, buf_size*8);


    if (s->context_initialized &&
        (s->width  != avctx->coded_width ||
         s->height != avctx->coded_height)) {
        vc1_decode_end(avctx);
    }

    if (!s->context_initialized) {
        if (ff_msmpeg4_decode_init(avctx) < 0 || vc1_decode_init_alloc_tables(v) < 0)
            return -1;

        s->low_delay = !avctx->has_b_frames || v->res_sprite;

        s->h_edge_pos = avctx->coded_width;
        s->v_edge_pos = avctx->coded_height;
    }

    /* We need to set current_picture_ptr before reading the header,
     * otherwise we cannot store anything in there. */
    if (s->current_picture_ptr == NULL || s->current_picture_ptr->f.data[0]) {
        int i = ff_find_unused_picture(s, 0);
        if (i < 0)
            goto err;
        s->current_picture_ptr = &s->picture[i];
    }

    // do parse frame header
    v->pic_header_flag = 0;

    if (v->profile < PROFILE_ADVANCED) {
			vc1_parse_frame_header(v, &s->gb);
    } else {
			vc1_parse_frame_header_adv(v, &s->gb);
    }

    if (v->field_mode && buf_start_second_field) {
        s->picture_structure = PICT_BOTTOM_FIELD - v->tff;
        *nFrameSize = buf_start_second_field - buf;
    } else {
        s->picture_structure = PICT_FRAME;
    }

end:
    av_free(buf2);
    for (i = 0; i < n_slices; i++)
        av_free(slices[i].buf);
    av_free(slices);
    return buf_size;

err:
    av_log(avctx, AV_LOG_ERROR, "Error in av_vc1_decode_frame()\n");
    av_free(buf2);
    for (i = 0; i < n_slices; i++)
        av_free(slices[i].buf);
    av_free(slices);
    return -1;
}

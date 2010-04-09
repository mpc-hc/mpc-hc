/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "avcodec.h"
#include "common.h"
#include "dsputil.h"

#define PNGFrameType_RGB24 0x01
#define PNGFrameType_YUY2  0x02
#define PNGFrameType_YV12  0x03

extern AVCodec png_decoder;

typedef struct CorePNGCodecPrivate {
    int16_t wSize;
    int8_t bType;
} CorePNGCodecPrivate;

typedef struct CorePNGcontext{
    AVCodecContext *avctx;
    DSPContext dsp;
    AVFrame picture,prev_picture;
    CorePNGCodecPrivate private;
    uint8_t *buf;int buf_size;
    int shiftX,shiftY;
    AVCodecContext *decctx;
    AVFrame decframe;
} CorePNGcontext;

static int read_image(CorePNGcontext * const a, uint8_t *ptr, int linesize,int flip)
{
    int got_picture=0,ret;
    a->decframe.data[0]=ptr;
    a->decframe.linesize[0]=linesize*(flip?-1:1);
    ret=avcodec_decode_video(a->decctx,&a->decframe,&got_picture,a->buf,a->buf_size);
    if(ret>0){
        a->buf+=ret;
        a->buf_size-=ret;
    }
    return 0;
}

static int decode_frame(AVCodecContext *avctx,
                        void *data, int *data_size,
                        uint8_t *buf, int buf_size)
{
    CorePNGcontext * const a = avctx->priv_data;
    int Bpp,num_planes,swapUV=0;
    AVFrame temp,*p;

    if (!a->decctx) {
        a->decctx=avcodec_alloc_context();
        avcodec_open(a->decctx,&png_decoder);
        avcodec_get_frame_defaults(&a->decframe);
    }

    temp= a->picture;
    a->picture= a->prev_picture;
    a->prev_picture= temp;

    p= &a->picture;
    avctx->coded_frame= p;

    avctx->flags |= CODEC_FLAG_EMU_EDGE; // alternatively we would have to use our own buffer management

    if(p->data[0])
        avctx->release_buffer(avctx, p);

    p->reference= 1;
    if(avctx->get_buffer(avctx, p) < 0){
        av_log(avctx, AV_LOG_ERROR, "get_buffer() failed\n");
        return -1;
    }

    /* special case for last picture */
    if (buf_size == 0) {
        return 0;
    }

    a->buf=buf;a->buf_size=buf_size;

    switch(a->private.bType){
    case PNGFrameType_RGB24:
        read_image(a,a->picture.data[0],a->picture.linesize[0],0);
        Bpp=3;num_planes=1;
        break;
    case PNGFrameType_YUY2:
        swapUV=1;
    case PNGFrameType_YV12:
        read_image(a,a->picture.data[0],a->picture.linesize[0],1);
        read_image(a,a->picture.data[swapUV?1:2],a->picture.linesize[swapUV?1:2],1);
        read_image(a,a->picture.data[swapUV?2:1],a->picture.linesize[swapUV?2:1],1);
        Bpp=1;num_planes=3;
        break;
    default:
        return 0;
    }

    if(avctx->sample_fmt==SAMPLE_I){ //indicates that this is a keyframe, CorePNG doesn't store this info in the stream ifself
        a->picture.key_frame=1;
        a->picture.pict_type=FF_I_TYPE;
    }else{
        int i,shiftX=0,shiftY=0;
        a->picture.key_frame=0;
        a->picture.pict_type=FF_P_TYPE;
        for(i=0;i<num_planes;i++,shiftX=a->shiftX,shiftY=a->shiftY){
            uint8_t *cur=a->picture.data[i],*prev=a->prev_picture.data[i];
            int y;
            for(y=0;y<avctx->height>>shiftY;y++,cur+=a->picture.linesize[i],prev+=a->prev_picture.linesize[i])
                a->dsp.add_bytes(cur,prev,avctx->width*Bpp>>shiftX);
        }
    }

    *data_size = sizeof(AVFrame);
    *(AVFrame*)data = a->picture;

    return buf_size;
}

static int decode_init(AVCodecContext *avctx){

    CorePNGcontext * const a = avctx->priv_data;

    dsputil_init(&a->dsp, avctx);

    a->avctx= avctx;
    a->picture.data[0]=a->prev_picture.data[0]=NULL;

    if(avctx->extradata_size == sizeof(CorePNGCodecPrivate))
        memcpy(&a->private,avctx->extradata,sizeof(CorePNGCodecPrivate));
    else{
        a->private.wSize = sizeof(CorePNGCodecPrivate);
        a->private.bType = PNGFrameType_RGB24;
    }

    switch(a->private.bType){
    case PNGFrameType_RGB24:
        avctx->pix_fmt = PIX_FMT_BGR24;
        break;
    case PNGFrameType_YUY2:
        avctx->pix_fmt = PIX_FMT_YUV422P;
        break;
    case PNGFrameType_YV12:
        avctx->pix_fmt = PIX_FMT_YUV420P;
        break;
    }
    avctx->has_b_frames=0;

    avcodec_get_chroma_sub_sample(avctx->pix_fmt, &a->shiftX, &a->shiftY);

    a->decctx=NULL;

    return 0;
}

static int decode_done(AVCodecContext *avctx)
{
    CorePNGcontext * const a = avctx->priv_data;
    if(a->decctx){
        avcodec_close(a->decctx);
        av_free(a->decctx);
    }
    return 0;
}

AVCodec corepng_decoder = {
    "corepng",
    CODEC_TYPE_VIDEO,
    CODEC_ID_COREPNG,
    sizeof(CorePNGcontext),
    decode_init,
    NULL,
    decode_done,
    decode_frame,
    /*.capabilities = */0,
    /*.next = */NULL,
    /*.flush = */NULL,
    /*.supported_framerates = */NULL,
    /*.pix_fmts = */NULL,
    /*.long_name = */"CorePNG",
};

/*
 * Provide registration of all codecs, parsers and bitstream filters for libavcodec.
 * Copyright (c) 2002 Fabrice Bellard
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * Provide registration of all codecs, parsers and bitstream filters for libavcodec.
 */

#include "avcodec.h"

#define REGISTER_ENCODER(X,x) { \
          extern AVCodec ff_##x##_encoder; \
          if(CONFIG_##X##_ENCODER)  avcodec_register(&ff_##x##_encoder); }
#define REGISTER_DECODER(X,x) { \
          extern AVCodec ff_##x##_decoder; \
          if(CONFIG_##X##_DECODER)  avcodec_register(&ff_##x##_decoder); }
#define REGISTER_ENCDEC(X,x)  REGISTER_ENCODER(X,x); REGISTER_DECODER(X,x)

#define REGISTER_PARSER(X,x) { \
          extern AVCodecParser ff_##x##_parser; \
          if(CONFIG_##X##_PARSER)  av_register_codec_parser(&ff_##x##_parser); }

void avcodec_register_all(void)
{
    static int initialized;

    if (initialized)
        return;
    initialized = 1;

    /* video codecs */
    //REGISTER_DECODER (AASC, aasc);
    REGISTER_DECODER (AMV, amv);
    //REGISTER_DECODER (ASV1, asv1);
    //REGISTER_DECODER (ASV2, asv2);
    //REGISTER_DECODER (AVS, avs);
    //REGISTER_DECODER (CAVS, cavs);
    //REGISTER_DECODER (CINEPAK, cinepak);
    //REGISTER_DECODER (CSCD, cscd);
    //REGISTER_DECODER (CYUV, cyuv);
    //REGISTER_ENCDEC  (DVVIDEO, dvvideo);
    //REGISTER_DECODER (EIGHTBPS, eightbps);
    //REGISTER_ENCDEC  (FFV1, ffv1);
    //REGISTER_ENCDEC  (FFVHUFF, ffvhuff);
    REGISTER_DECODER (FLV, flv);
    //REGISTER_DECODER (FRAPS, fraps);
    //REGISTER_ENCDEC  (H261, h261);
    REGISTER_DECODER (H263, h263);
    //REGISTER_DECODER (H263I, h263i);
    //REGISTER_ENCODER (H263P, h263p);
    REGISTER_DECODER (H264, h264);
    //REGISTER_ENCDEC  (HUFFYUV, huffyuv);
    //REGISTER_DECODER (INDEO2, indeo2);
    REGISTER_DECODER (INDEO3, indeo3);
    REGISTER_DECODER (INDEO4, indeo4);
    REGISTER_DECODER (INDEO5, indeo5);
    //REGISTER_DECODER (JPEGLS, jpegls);
    //REGISTER_ENCODER (LJPEG, ljpeg);
    //REGISTER_DECODER (LOCO, loco);
    //REGISTER_ENCDEC  (MJPEG, mjpeg);
    REGISTER_DECODER (MJPEG, mjpeg);
    REGISTER_DECODER (MJPEGB, mjpegb);
    //REGISTER_ENCDEC  (MPEG1VIDEO, mpeg1video);
    REGISTER_DECODER (MPEG2VIDEO, mpeg2video);
    REGISTER_DECODER (MPEG4, mpeg4);
    //REGISTER_DECODER (MPEGVIDEO, mpegvideo);
    REGISTER_DECODER (MSMPEG4V1, msmpeg4v1);
    REGISTER_DECODER (MSMPEG4V2, msmpeg4v2);
    REGISTER_DECODER (MSMPEG4V3, msmpeg4v3);
    //REGISTER_DECODER (MSRLE, msrle);
    //REGISTER_DECODER (MSVIDEO1, msvideo1);
    //REGISTER_DECODER (MSZH, mszh);
    //REGISTER_ENCDEC  (PNG, png);
    //REGISTER_DECODER (QPEG, qpeg);
    //REGISTER_DECODER (QTRLE, qtrle);
    //REGISTER_DECODER (RPZA, rpza);
    REGISTER_DECODER (RV10, rv10);
    REGISTER_DECODER (RV20, rv20);
    REGISTER_DECODER (RV30, rv30);
    REGISTER_DECODER (RV40, rv40);
    //REGISTER_DECODER (SP5X, sp5x);
    REGISTER_DECODER (SVQ1, svq1);
    REGISTER_DECODER (SVQ3, svq3);
    REGISTER_DECODER (THEORA, theora);
    //REGISTER_DECODER (TRUEMOTION1, truemotion1);
    //REGISTER_DECODER (TRUEMOTION2, truemotion2);
    REGISTER_DECODER (TSCC, tscc);
    //REGISTER_DECODER (ULTI, ulti);
    REGISTER_DECODER (VC1, vc1);
    //REGISTER_DECODER (VC1IMAGE, vc1image);
    //REGISTER_DECODER (VCR1, vcr1);
    REGISTER_DECODER (VP3, vp3);
    REGISTER_DECODER (VP5, vp5);
    REGISTER_DECODER (VP6, vp6);
    REGISTER_DECODER (VP6A, vp6a);
    REGISTER_DECODER (VP6F, vp6f);
    REGISTER_DECODER (VP8, vp8);
    REGISTER_DECODER (WMV1, wmv1);
    REGISTER_DECODER (WMV2, wmv2);
    REGISTER_DECODER (WMV3, wmv3);
    //REGISTER_DECODER (WMV3IMAGE, wmv3image);
    //REGISTER_DECODER (WNV1, wnv1);
    //REGISTER_DECODER (XL, xl);
    //REGISTER_DECODER (ZLIB, zlib);
    //REGISTER_DECODER (ZMBV, zmbv);

    /* audio codecs */
    REGISTER_DECODER (AAC, aac);
    REGISTER_DECODER (AAC_LATM, aac_latm);
    REGISTER_DECODER (AC3, ac3);
    REGISTER_DECODER (ATRAC3, atrac3);
    REGISTER_DECODER (COOK, cook);
    //REGISTER_DECODER (DCA, dca);
    REGISTER_DECODER (EAC3, eac3);
    //REGISTER_DECODER (FLAC, flac);
    //REGISTER_DECODER (GSM, gsm);
    //REGISTER_DECODER (GSM_MS, gsm_ms);
    //REGISTER_DECODER (IMC, imc);
    //REGISTER_DECODER (MACE3, mace3);
    //REGISTER_DECODER (MACE6, mace6);
    REGISTER_DECODER (MLP, mlp);
    //REGISTER_DECODER (MP1, mp1);
    REGISTER_DECODER (MP1FLOAT, mp1float);
    //REGISTER_DECODER (MP2, mp2);
    REGISTER_DECODER (MP2FLOAT, mp2float);
    //REGISTER_DECODER (MP3, mp3);
    REGISTER_DECODER (MP3FLOAT, mp3float);
    REGISTER_DECODER (NELLYMOSER, nellymoser);
    //REGISTER_DECODER (QDM2, qdm2);
    REGISTER_DECODER (RA_144, ra_144);
    REGISTER_DECODER (RA_288, ra_288);
    REGISTER_DECODER (SIPR, sipr);
    REGISTER_DECODER (TRUEHD, truehd);
    //REGISTER_DECODER (TRUESPEECH, truespeech);
    //REGISTER_DECODER (TTA, tta);
    REGISTER_DECODER (VORBIS, vorbis);
    //REGISTER_DECODER (WMAV1, wmav1);
    //REGISTER_DECODER (WMAV2, wmav2);
    //REGISTER_DECODER (WAVPACK, wavpack);
    REGISTER_DECODER (AMRNB, amrnb);
    REGISTER_DECODER (AMRWB, amrwb);

    //REGISTER_ENCODER (AC3, ac3);
    //REGISTER_ENCODER (AC3_FIXED, ac3_fixed);

    /* PCM codecs */
    //REGISTER_DECODER (PCM_ALAW, pcm_alaw);
    //REGISTER_DECODER (PCM_MULAW,pcm_mulaw);

    /* ADPCM codecs */
    REGISTER_DECODER (ADPCM_4XM, adpcm_4xm);
    REGISTER_DECODER (ADPCM_CT, adpcm_ct);
    REGISTER_DECODER (ADPCM_EA, adpcm_ea);
    //REGISTER_DECODER (ADPCM_G726, adpcm_g726);
    REGISTER_DECODER (ADPCM_IMA_AMV, adpcm_ima_amv);
    REGISTER_DECODER (ADPCM_IMA_DK3, adpcm_ima_dk3);
    REGISTER_DECODER (ADPCM_IMA_DK4, adpcm_ima_dk4);
    REGISTER_DECODER (ADPCM_IMA_QT, adpcm_ima_qt);
    REGISTER_DECODER (ADPCM_IMA_SMJPEG, adpcm_ima_smjpeg);
    REGISTER_DECODER (ADPCM_IMA_WAV, adpcm_ima_wav);
    REGISTER_DECODER (ADPCM_IMA_WS, adpcm_ima_ws);
    REGISTER_DECODER (ADPCM_MS, adpcm_ms);
    REGISTER_DECODER (ADPCM_SBPRO_2, adpcm_sbpro_2);
    REGISTER_DECODER (ADPCM_SBPRO_3, adpcm_sbpro_3);
    REGISTER_DECODER (ADPCM_SBPRO_4, adpcm_sbpro_4);
    REGISTER_DECODER (ADPCM_SWF, adpcm_swf);
    REGISTER_DECODER (ADPCM_XA, adpcm_xa);
    REGISTER_DECODER (ADPCM_YAMAHA, adpcm_yamaha);

    /* parsers */
    //REGISTER_PARSER  (AAC, aac);
    REGISTER_PARSER (AC3, ac3);
    //REGISTER_PARSER  (DCA, dca);
    REGISTER_PARSER (MLP, mlp);
    REGISTER_PARSER (MPEGAUDIO, mpegaudio);
    REGISTER_PARSER  (H264, h264);
    REGISTER_PARSER  (MPEGVIDEO, mpegvideo);
}

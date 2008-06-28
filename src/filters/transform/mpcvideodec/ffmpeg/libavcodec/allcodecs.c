/*
 * Provides registration of all codecs for libavcodec.
 * Copyright (c) 2002 Fabrice Bellard.
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
 * @file allcodecs.c
 * Provides registration of all codecs for libavcodec.
 */

#include "avcodec.h"

#define REGISTER_DECODER(x) { \
          extern AVCodec x##_decoder; \
          register_avcodec(&x##_decoder); }
#define REGISTER_PARSER(X,x) { \
          extern AVCodecParser x##_parser; \
          av_register_codec_parser(&x##_parser); }

/**
 * Register all the codecs which were enabled at configuration time. 
 * If you do not call this function you can select exactly which formats
 * you want to support, by using the individual registration functions.
 *
 * @see register_avcodec
 */
void avcodec_register_all(void)
{
	static int initialized;

  if (initialized)
    return;
  initialized = 1;

  /* video codecs */
//REGISTER_DECODER  (aasc);
  REGISTER_DECODER  (amv);
//REGISTER_DECODER  (asv1);
//REGISTER_DECODER  (asv2);
//REGISTER_DECODER  (avs);
//REGISTER_DECODER  (cavs);
//REGISTER_DECODER  (cinepak);
//REGISTER_DECODER  (cscd);
//REGISTER_DECODER  (corepng);
//REGISTER_DECODER  (cyuv);
//REGISTER_DECODER  (dvvideo);
//REGISTER_DECODER  (eightbps);
//REGISTER_DECODER  (ffv1);
//REGISTER_DECODER  (ffvhuff);
  REGISTER_DECODER  (flv);
//REGISTER_DECODER  (fraps);
//REGISTER_DECODER  (h261);
  REGISTER_DECODER  (h263);
//REGISTER_DECODER  (h263i);
  REGISTER_DECODER  (h264);
//REGISTER_DECODER  (huffyuv);
//REGISTER_DECODER  (indeo2);
//REGISTER_DECODER  (indeo3);
//REGISTER_DECODER  (jpegls);
//REGISTER_DECODER  (loco);
//REGISTER_DECODER  (mjpeg);
//REGISTER_DECODER  (mjpegb);
//REGISTER_DECODER  (mpeg1video);
//REGISTER_DECODER  (mpeg2video);
  REGISTER_DECODER  (mpeg4);
//REGISTER_DECODER  (mpegvideo);
  REGISTER_DECODER  (msmpeg4v1);
  REGISTER_DECODER  (msmpeg4v2);
  REGISTER_DECODER  (msmpeg4v3);
//REGISTER_DECODER  (msrle);
//REGISTER_DECODER  (msvideo1);
//REGISTER_DECODER  (mszh);
//#ifdef CONFIG_ZLIB
//REGISTER_DECODER  (png);
//#endif
//REGISTER_DECODER  (qpeg);
//REGISTER_DECODER  (qtrle);
//REGISTER_DECODER  (rpza);
//REGISTER_DECODER  (rv10);
//REGISTER_DECODER  (rv20);
  REGISTER_DECODER  (svq1);
  REGISTER_DECODER  (svq3);
  REGISTER_DECODER  (theora);
//REGISTER_DECODER  (truemotion1);
//REGISTER_DECODER  (truemotion2);
//REGISTER_DECODER  (tscc);
//REGISTER_DECODER  (ulti);
  REGISTER_DECODER  (vc1);
//REGISTER_DECODER  (vcr1);
//REGISTER_DECODER  (vp3);
  REGISTER_DECODER  (vp5);
  REGISTER_DECODER  (vp6);
  REGISTER_DECODER  (vp6a);
  REGISTER_DECODER  (vp6f);
  REGISTER_DECODER  (wmv1);
  REGISTER_DECODER  (wmv2);
  REGISTER_DECODER  (wmv3);
//REGISTER_DECODER  (wnv1);
//REGISTER_DECODER  (xl);
//REGISTER_DECODER  (zlib);
//REGISTER_DECODER  (zmbv);

  /* audio codecs */
//REGISTER_DECODER    (atrac3);
//REGISTER_DECODER    (amr_nb);
//REGISTER_DECODER    (cook);
//REGISTER_DECODER    (flac);
//#ifdef __GNUC__
//REGISTER_DECODER    (imc);
//#endif
//REGISTER_DECODER    (mace3);
//REGISTER_DECODER    (mace6);
//REGISTER_DECODER    (msgsm);
//REGISTER_DECODER    (qdm2);
//#ifdef __GNUC__
//REGISTER_DECODER    (ra_144);
//REGISTER_DECODER    (ra_288);
//#endif
//REGISTER_DECODER    (truespeech);
//REGISTER_DECODER    (tta);

/* pcm codecs */
//REGISTER_DECODER  (pcm_alaw);
//REGISTER_DECODER  (pcm_mulaw);

/* adpcm codecs */
//REGISTER_DECODER  (adpcm_4xm);
//REGISTER_DECODER  (adpcm_ct);
//REGISTER_DECODER  (adpcm_ea);
//REGISTER_DECODER  (adpcm_g726);
//REGISTER_DECODER  (adpcm_ima_amv);
//REGISTER_DECODER  (adpcm_ima_dk3);
//REGISTER_DECODER  (adpcm_ima_dk4);
//REGISTER_DECODER  (adpcm_ima_qt);
//REGISTER_DECODER  (adpcm_ima_smjpeg);
//REGISTER_DECODER  (adpcm_ima_wav);
//REGISTER_DECODER  (adpcm_ima_ws);
//REGISTER_DECODER  (adpcm_ms);
//REGISTER_DECODER  (adpcm_sbpro_2);
//REGISTER_DECODER  (adpcm_sbpro_3);
//REGISTER_DECODER  (adpcm_sbpro_4);
//REGISTER_DECODER  (adpcm_swf);
//REGISTER_DECODER  (adpcm_xa);
//REGISTER_DECODER  (adpcm_yamaha);

/* parsers */
//REGISTER_PARSER  (AAC, aac);
//REGISTER_PARSER  (AC3, ac3);
//REGISTER_PARSER  (CAVSVIDEO, cavsvideo);
//REGISTER_PARSER  (DCA, dca);
//REGISTER_PARSER  (DVBSUB, dvbsub);
//REGISTER_PARSER  (DVDSUB, dvdsub);
//REGISTER_PARSER  (H261, h261);
//REGISTER_PARSER  (H263, h263);
//REGISTER_PARSER  (H264, h264);
//REGISTER_PARSER  (MJPEG, mjpeg);
//REGISTER_PARSER  (MLP, mlp);
//REGISTER_PARSER  (MPEG4VIDEO, mpeg4video);
//REGISTER_PARSER  (MPEGAUDIO, mpegaudio);
//REGISTER_PARSER  (MPEGVIDEO, mpegvideo);
//REGISTER_PARSER  (PNM, pnm);
//REGISTER_PARSER  (VC1, vc1);
}

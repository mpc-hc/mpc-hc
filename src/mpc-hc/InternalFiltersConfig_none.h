/*
 * $Id$
 *
 * (C) 2012 see Authors.txt
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

// Internal source filters
#define INTERNAL_SOURCEFILTER_AVI       0
#define INTERNAL_SOURCEFILTER_CDDA      0
#define INTERNAL_SOURCEFILTER_CDXA      0
#define INTERNAL_SOURCEFILTER_DSM       0
#define INTERNAL_SOURCEFILTER_DTSAC3    0
#define INTERNAL_SOURCEFILTER_VTS       0
#define INTERNAL_SOURCEFILTER_DVSOURCE  0
#define INTERNAL_SOURCEFILTER_FLIC      0
#define INTERNAL_SOURCEFILTER_FLAC      0
#define INTERNAL_SOURCEFILTER_FLV       0
#define INTERNAL_SOURCEFILTER_MATROSKA  0
#define INTERNAL_SOURCEFILTER_MP4       0
#define INTERNAL_SOURCEFILTER_OGG       0
#define INTERNAL_SOURCEFILTER_MPEGAUDIO 0
#define INTERNAL_SOURCEFILTER_MPEG      0
#define INTERNAL_SOURCEFILTER_REALMEDIA 0
#define INTERNAL_SOURCEFILTER_SHOUTCAST 0
#define INTERNAL_SOURCEFILTER_UDP       0
#define INTERNAL_SOURCEFILTER_AVI2AC3   0

// Internal audio decoders
#define INTERNAL_DECODER_DTS            0
#define INTERNAL_DECODER_LPCM           0
#define INTERNAL_DECODER_PS2AUDIO       0
#define INTERNAL_DECODER_REALAUDIO      0
#define INTERNAL_DECODER_FLAC           0
#define INTERNAL_DECODER_PCM            0

// Internal audio decoders (FFmpeg)
#define INTERNAL_DECODER_AC3            0 /* also E-AC3,TrueHD,MLP */
#define INTERNAL_DECODER_AAC            0
#define INTERNAL_DECODER_ALAC           0
#define INTERNAL_DECODER_ALS            0
#define INTERNAL_DECODER_MPEGAUDIO      0
#define INTERNAL_DECODER_VORBIS         0
#define INTERNAL_DECODER_NELLYMOSER     0
#define INTERNAL_DECODER_AMR            0
#define INTERNAL_DECODER_ADPCM          0

// Internal video decoders
#define INTERNAL_DECODER_MPEG1          0
#define INTERNAL_DECODER_MPEG2          0
#define INTERNAL_DECODER_REALVIDEO      0

// Internal video decoders (FFmpeg)
#define INTERNAL_DECODER_H264           0
#define INTERNAL_DECODER_VC1            0
#define INTERNAL_DECODER_FLV            0
#define INTERNAL_DECODER_VP356          0
#define INTERNAL_DECODER_DIVX           0
#define INTERNAL_DECODER_XVID           0
#define INTERNAL_DECODER_WMV            0
#define INTERNAL_DECODER_MSMPEG4        0
#define INTERNAL_DECODER_SVQ            0
#define INTERNAL_DECODER_H263           0
#define INTERNAL_DECODER_THEORA         0
#define INTERNAL_DECODER_AMVV           0
#define INTERNAL_DECODER_VP8            0
#define INTERNAL_DECODER_MJPEG          0
#define INTERNAL_DECODER_INDEO          0

// DXVA decoders
#define INTERNAL_DECODER_H264_DXVA      0
#define INTERNAL_DECODER_VC1_DXVA       0
#define INTERNAL_DECODER_MPEG2_DXVA     0
#define INTERNAL_DECODER_WMV3_DXVA      0

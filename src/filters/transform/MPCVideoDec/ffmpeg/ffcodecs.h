#ifndef _FFCODECS_H_
#define _FFCODECS_H_

#ifdef __cplusplus

template<> struct isPOD<CodecID> {
    enum {is=true};
};

const FOURCC* getCodecFOURCCs(CodecID codecId);
//const char_t* getCodecName(CodecID codecId);

static __inline bool lavc_codec(int x)
{
    return x>0 && x<200;
}
static __inline bool raw_codec(int x)
{
    return x>=300 && x<400;
}
static __inline bool xvid_codec(int x)
{
    return x==CODEC_ID_XVID4;
}
static __inline bool wmv9_codec(int x)
{
    return x>=1000 && x<1100;
}
static __inline bool mpeg12_codec(int x)
{
    return x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_LIBMPEG2;
}
static __inline bool mpeg1_codec(int x)
{
    return x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_LIBMPEG2;
}
static __inline bool mpeg2_codec(int x)
{
    return x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_LIBMPEG2;
}
static __inline bool mpeg4_codec(int x)
{
    return x==CODEC_ID_MPEG4 || xvid_codec(x);
}
static __inline bool spdif_codec(int x)
{
    return x==CODEC_ID_SPDIF_AC3 || x==CODEC_ID_SPDIF_DTS;
}
static __inline bool bitstream_codec(int x)
{
    return x==CODEC_ID_BITSTREAM_TRUEHD || x==CODEC_ID_BITSTREAM_DTSHD || x==CODEC_ID_BITSTREAM_EAC3;
}
static __inline bool huffyuv_codec(int x)
{
    return x==CODEC_ID_HUFFYUV || x==CODEC_ID_FFVHUFF;
}
static __inline bool lossless_codec(int x)
{
    return huffyuv_codec(x) || x==CODEC_ID_LJPEG || x==CODEC_ID_FFV1 || x==CODEC_ID_DVVIDEO;
}
static __inline bool h264_codec(int x)
{
    return x == CODEC_ID_H264 || x == CODEC_ID_H264_MT;
}
static __inline bool vc1_codec(int x)
{
    return x == CODEC_ID_VC1 || x == CODEC_ID_WMV9_LIB;
}
static __inline bool dxva_codec(int x)
{
    return x == CODEC_ID_H264_DXVA || x == CODEC_ID_VC1_DXVA;
}

//I'm not sure of all these
static __inline bool sup_CBR(int x)
{
    return !lossless_codec(x) && !raw_codec(x);
}
static __inline bool sup_VBR_QUAL(int x)
{
    return !lossless_codec(x) && !raw_codec(x);
}
static __inline bool sup_VBR_QUANT(int x)
{
    return (lavc_codec(x) || xvid_codec(x)) && !lossless_codec(x);
}
static __inline bool sup_XVID2PASS(int x)
{
    return sup_VBR_QUANT(x);
}
static __inline bool sup_LAVC2PASS(int x)
{
    return (lavc_codec(x) && !lossless_codec(x) && x!=CODEC_ID_MJPEG && !raw_codec(x));
}

static __inline bool sup_interlace(int x)
{
    return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x);
}
static __inline bool sup_gray(int x)
{
    return x!=CODEC_ID_LJPEG && x!=CODEC_ID_FFV1 && !wmv9_codec(x) && !raw_codec(x) && x!=CODEC_ID_DVVIDEO;
}
static __inline bool sup_globalheader(int x)
{
    return x==CODEC_ID_MPEG4;
}
static __inline bool sup_part(int x)
{
    return x==CODEC_ID_MPEG4;
}
static __inline bool sup_packedBitstream(int x)
{
    return xvid_codec(x);
}
static __inline bool sup_minKeySet(int x)
{
    return x!=CODEC_ID_MJPEG && !lossless_codec(x) && !wmv9_codec(x) && !raw_codec(x);
}
static __inline bool sup_maxKeySet(int x)
{
    return x!=CODEC_ID_MJPEG && !lossless_codec(x) && !raw_codec(x);
}
static __inline bool sup_bframes(int x)
{
    return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x);
}
static __inline bool sup_adaptiveBframes(int x)
{
    return lavc_codec(x);
}
static __inline bool sup_closedGop(int x)
{
    return sup_bframes(x);
}
static __inline bool sup_lavcme(int x)
{
    return lavc_codec(x) && x!=CODEC_ID_MJPEG && !lossless_codec(x);
}
static __inline bool sup_quantProps(int x)
{
    return !lossless_codec(x) && !wmv9_codec(x) && !raw_codec(x);
}
static __inline bool sup_trellisQuant(int x)
{
    return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_XVID4 || x==CODEC_ID_H263 || x==CODEC_ID_H263P;
}
static __inline bool sup_masking(int x)
{
    return x==CODEC_ID_MPEG4 || x==CODEC_ID_H263 || x==CODEC_ID_H263P || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x);
}
static __inline bool sup_lavcOnePass(int x)
{
    return (lavc_codec(x) && !lossless_codec(x));
}
static __inline bool sup_perFrameQuant(int x)
{
    return !lossless_codec(x) && !wmv9_codec(x) && !raw_codec(x);
}
static __inline bool sup_4mv(int x)
{
    return x==CODEC_ID_MPEG4 || x==CODEC_ID_H263 || x==CODEC_ID_H263P;
}
static __inline bool sup_aspect(int x)
{
    return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO;
}
static __inline bool sup_PSNR(int x)
{
    return (lavc_codec(x) && !lossless_codec(x)) || xvid_codec(x);
}
static __inline bool sup_quantBias(int x)
{
    return lavc_codec(x) && !lossless_codec(x);
}
static __inline bool sup_MPEGquant(int x)
{
    return x==CODEC_ID_MPEG4 || x==CODEC_ID_MSMPEG4V3 || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x);
}
static __inline bool sup_lavcQuant(int x)
{
    return lavc_codec(x) && sup_quantProps(x);
}
static __inline bool sup_customQuantTables(int x)
{
    return x==CODEC_ID_MPEG4 || xvid_codec(x) || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO;
}
static __inline bool sup_qpel(int x)
{
    return x==CODEC_ID_MPEG4 || xvid_codec(x);
}
static __inline bool sup_gmc(int x)
{
    return xvid_codec(x);
}
static __inline bool sup_me_mv0(int x)
{
    return sup_lavcme(x);
}
static __inline bool sup_cbp_rd(int x)
{
    return x==CODEC_ID_MPEG4;
}
static __inline bool sup_qns(int x)
{
    return lavc_codec(x) && sup_quantProps(x) && x!=CODEC_ID_MSMPEG4V3 && x!=CODEC_ID_MSMPEG4V2 && x!=CODEC_ID_MSMPEG4V1 && x!=CODEC_ID_WMV1 && x!=CODEC_ID_WMV2 && x!=CODEC_ID_MJPEG;
}
static __inline bool sup_threads(int x)
{
    return x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_XVID4;
}
static __inline bool sup_threads_dec(int x)
{
    return x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO;
}
static __inline bool sup_palette(int x)
{
    return x==CODEC_ID_MSVIDEO1 || x==CODEC_ID_8BPS || x==CODEC_ID_QTRLE || x==CODEC_ID_TSCC || x==CODEC_ID_QPEG || x==CODEC_ID_PNG;
}

#endif

#endif

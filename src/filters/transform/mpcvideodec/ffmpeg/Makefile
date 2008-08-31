LAVC_DIR=libavcodec
LAVU_DIR=libavutil
LAVF_DIR=libavformat
ZLIB_DIR=../../../../zlib
PNG_DIR =../../../../libpng

CFLAGS+= -I. -I.. -I$(LAVC_DIR) -I$(LAVU_DIR) -I$(LAVF_DIR) -I../codecs -I../imgFilters -I$(ZLIB_DIR) -I$(PNG_DIR) \
		-DHAVE_AV_CONFIG_H -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -std=gnu99 -fasm

SRCS_C=\
		$(LAVC_DIR)/allcodecs.c \
		$(LAVC_DIR)/bitstream.c \
		$(LAVC_DIR)/cabac.c \
		$(LAVC_DIR)/CompilatorVersion.c\
		$(LAVC_DIR)/dsputil.c \
		$(LAVC_DIR)/error_resilience.c \
		$(LAVC_DIR)/golomb.c \
		$(LAVC_DIR)/h263.c \
		$(LAVC_DIR)/h263_parser.c \
		$(LAVC_DIR)/h263dec.c \
		$(LAVC_DIR)/h264.c \
		$(LAVC_DIR)/h264_parser.c \
		$(LAVC_DIR)/h264idct.c \
		$(LAVC_DIR)/h264pred.c \
		$(LAVC_DIR)/huffman.c \
		$(LAVC_DIR)/imgconvert.c \
		$(LAVC_DIR)/intrax8.c \
		$(LAVC_DIR)/intrax8dsp.c \
		$(LAVC_DIR)/jpegls.c \
		$(LAVC_DIR)/jpeglsdec.c \
		$(LAVC_DIR)/jrevdct.c \
		$(LAVC_DIR)/mjpeg.c \
		$(LAVC_DIR)/mjpegdec.c \
		$(LAVC_DIR)/mpeg4video_parser.c \
		$(LAVC_DIR)/mpegvideo.c \
		$(LAVC_DIR)/msmpeg4.c \
		$(LAVC_DIR)/msmpeg4data.c \
		$(LAVC_DIR)/parser.c \
		$(LAVC_DIR)/simple_idct.c \
		$(LAVC_DIR)/sp5xdec.c \
		$(LAVC_DIR)/svq1.c \
		$(LAVC_DIR)/svq1dec.c \
		$(LAVC_DIR)/utils.c \
		$(LAVC_DIR)/vc1.c \
		$(LAVC_DIR)/vc1_parser.c \
		$(LAVC_DIR)/vc1data.c \
		$(LAVC_DIR)/vc1dsp.c \
		$(LAVC_DIR)/vp3.c \
		$(LAVC_DIR)/vp3dsp.c \
		$(LAVC_DIR)/vp5.c \
		$(LAVC_DIR)/vp56.c \
		$(LAVC_DIR)/vp56data.c \
		$(LAVC_DIR)/vp6.c \
		$(LAVC_DIR)/w32thread.c \
		$(LAVC_DIR)/wmv2.c \
		$(LAVC_DIR)/wmv2dec.c \
		$(LAVC_DIR)/xiph.c	

#SRCS_C+=\
#		$(LAVC_DIR)/8bps.c \
#		$(LAVC_DIR)/aac_ac3_parser.c \
#		$(LAVC_DIR)/aac_parser.c \
#		$(LAVC_DIR)/aasc.c \
#		$(LAVC_DIR)/ac3.c \
#		$(LAVC_DIR)/ac3_parser.c \
#		$(LAVC_DIR)/ac3enc.c \
#		$(LAVC_DIR)/ac3tab.c \
#		$(LAVC_DIR)/adpcm.c \
#		$(LAVC_DIR)/libamr.c \
#		$(LAVC_DIR)/amr_float/interf_dec.c \
#		$(LAVC_DIR)/amr_float/sp_dec.c \
#		$(LAVC_DIR)/atrac3.c \
#		$(LAVC_DIR)/asv1.c \
#		$(LAVC_DIR)/avs.c \
#		$(LAVC_DIR)/cavs.c \
#		$(LAVC_DIR)/cavsdec.c \
#		$(LAVC_DIR)/cavsdsp.c \
#		$(LAVC_DIR)/cinepak.c \
#		$(LAVC_DIR)/cook.c \
#		$(LAVC_DIR)/corepng.c \
#		$(LAVC_DIR)/cscd.c \
#		$(LAVC_DIR)/cyuv.c \
#		$(LAVC_DIR)/dv.c \
#		$(LAVC_DIR)/eval.c \
#		$(LAVC_DIR)/faanidct.c \
#		$(LAVC_DIR)/fft.c \
#		$(LAVC_DIR)/ffv1.c \
#		$(LAVC_DIR)/flac.c \
#		$(LAVC_DIR)/fraps.c \
#		$(LAVC_DIR)/g726.c \
#		$(LAVC_DIR)/gsm.c \
#		$(LAVC_DIR)/h261.c \
#		$(LAVC_DIR)/h261_parser.c \
#		$(LAVC_DIR)/h261dec.c \
#		$(LAVC_DIR)/huffyuv.c \
#		$(LAVC_DIR)/imc.c \
#		$(LAVC_DIR)/indeo2.c \
#		$(LAVC_DIR)/indeo3.c \
#		$(LAVC_DIR)/jfdctfst.c \
#		$(LAVC_DIR)/jfdctint.c \
#		$(LAVC_DIR)/lcldec.c \
#		$(LAVC_DIR)/loco.c \
#		$(LAVC_DIR)/mace.c \
#		$(LAVC_DIR)/mdct.c \
#		$(LAVC_DIR)/mjpegbdec.c \
#		$(LAVC_DIR)/mjpeg_parser.c \
#		$(LAVC_DIR)/mlp_parser.c \
#		$(LAVC_DIR)/mpeg12.c \
#		$(LAVC_DIR)/mpeg12data.c \
#		$(LAVC_DIR)/mpeg4audio.c \
#		$(LAVC_DIR)/mpegaudio_parser.c \
#		$(LAVC_DIR)/mpegaudiodata.c \
#		$(LAVC_DIR)/mpegaudiodec.c \
#		$(LAVC_DIR)/mpegaudiodecheader.c \
#		$(LAVC_DIR)/mpegvideo_parser.c \
#		$(LAVC_DIR)/msrle.c \
#		$(LAVC_DIR)/msvideo1.c \
#		$(LAVC_DIR)/nellymoserdec.c \
#		$(LAVC_DIR)/opt.c \
#		$(LAVC_DIR)/pcm.c \
#		$(LAVC_DIR)/png.c \
#		$(LAVC_DIR)/pngdec.c \
#		$(LAVC_DIR)/rv10.c \
#		$(LAVC_DIR)/qdm2.c \
#		$(LAVC_DIR)/qpeg.c \
#		$(LAVC_DIR)/qtrle.c \
#		$(LAVC_DIR)/ra144.c \
#		$(LAVC_DIR)/ra288.c \
#		$(LAVC_DIR)/rpza.c \
#		$(LAVC_DIR)/rtjpeg.c \
#		$(LAVC_DIR)/truemotion1.c \
#		$(LAVC_DIR)/truemotion2.c \
#		$(LAVC_DIR)/truespeech.c \
#		$(LAVC_DIR)/tscc.c \
#		$(LAVC_DIR)/tta.c \
#		$(LAVC_DIR)/ulti.c \
#		$(LAVC_DIR)/vcr1.c \
#		$(LAVC_DIR)/vorbis.c \
#		$(LAVC_DIR)/vorbis_dec.c \
#		$(LAVC_DIR)/vorbis_data.c \
#		$(LAVC_DIR)/wma.c \
#		$(LAVC_DIR)/wmadec.c \
#		$(LAVC_DIR)/wnv1.c \
#		$(LAVC_DIR)/xl.c \
#		$(LAVC_DIR)/zmbv.c

SRCS_C+=\
		$(LAVU_DIR)/crc.c \
		$(LAVU_DIR)/integer.c \
		$(LAVU_DIR)/intfloat_readwrite.c \
		$(LAVU_DIR)/log.c \
		$(LAVU_DIR)/mathematics.c \
		$(LAVU_DIR)/mem.c \
		$(LAVU_DIR)/rational.c

#SRCS_C+=\
#		$(LAVU_DIR)/lzo.c \
#		$(LAVU_DIR)/random.c \
#		$(LAVU_DIR)/string.c

#SRCS_C+=\
#		$(LAVF_DIR)/allformats.c \
#		$(LAVF_DIR)/avio.c \
#		$(LAVF_DIR)/aviobuf.c \
#		$(LAVF_DIR)/avisynth.c \
#		$(LAVF_DIR)/cutils.c \
#		$(LAVF_DIR)/file.c \
#		$(LAVF_DIR)/matroska.c \
#		$(LAVF_DIR)/matroskadec.c \
#		$(LAVF_DIR)/mpeg.c \
#		$(LAVF_DIR)/mpegts.c \
#		$(LAVF_DIR)/raw.c \
#		$(LAVF_DIR)/riff.c \
#		$(LAVF_DIR)/utils.c \

SRCS_C+=\
    $(LAVC_DIR)/i386/cpuid.c \
		$(LAVC_DIR)/i386/dsputil_mmx.c \
		$(LAVC_DIR)/i386/idct_mmx.c \
		$(LAVC_DIR)/i386/idct_mmx_xvid.c \
		$(LAVC_DIR)/i386/idct_sse2_xvid.c \
		$(LAVC_DIR)/i386/mpegvideo_mmx.c \
		$(LAVC_DIR)/i386/simple_idct_mmx.c \
		$(LAVC_DIR)/i386/vc1dsp_mmx.c \
		$(LAVC_DIR)/i386/vp3dsp_mmx.c \
		$(LAVC_DIR)/i386/vp3dsp_sse2.c
		
#SRCS_C+=\
#		$(LAVC_DIR)/i386/cavsdsp_mmx.c \
#		$(LAVC_DIR)/i386/fdct_mmx.c \
#		$(LAVC_DIR)/i386/fft_3dn.c \
#		$(LAVC_DIR)/i386/fft_3dn2.c \
#		$(LAVC_DIR)/i386/fft_sse.c

#SRCS_YASM=\
#      $(LAVC_DIR)/i386/fft_mmx.asm


ifeq ($(64BIT),yes)
	CFLAGS+=-DARCH_X86_64
	SLIB=../../../../../../../lib64/libavcodec_gcc.lib
else
	CFLAGS+=-DARCH_X86_32
	SLIB=../../../../../lib/libavcodec_gcc.lib
endif

DEF=libavcodec_dec.def

NOINTRIN=1

include makefile_c.inc

OPTFLAGS+= -fno-math-errno

.all: $(SLIB)

clean:
	rm -f *.o *.d *~ $(LAVC_DIR)/*.o $(LAVC_DIR)/*.d $(LAVC_DIR)/i386/*.o $(LAVC_DIR)/amr_float/*.o $(LAVF_DIR)/*.o $(LAVF_DIR)/*.d $(LAVU_DIR)/*.o $(LAVU_DIR)/*.d $(SLIB) $(ZLIB_DIR)/*.o $(PNG_DIR)/*.o $(LAVC_DIR)/i386/*.d $(LAVC_DIR)/amr_float/*.d $(ZLIB_DIR)/*.d

#
# include dependency files if they exist
#
-include $(SRCS:.cpp=.d) $(SRCS_C:.c=.d)

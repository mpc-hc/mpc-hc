LAVC_DIR = libavcodec
LAVU_DIR = libavutil
LSWS_DIR = libswscale
PNG_DIR  = ../../../../thirdparty/libpng
ZLIB_DIR = ../../../../thirdparty/zlib

ifeq ($(64BIT),yes)
    OUT_DIR =../../../../../bin/obj/Release_x64/libavcodec_gcc/
    SLIB_DIR=../../../../../bin/lib/Release_x64/
else
    OUT_DIR =../../../../../bin/obj/Release_Win32/libavcodec_gcc/
    SLIB_DIR=../../../../../bin/lib/Release_Win32/
endif

ifeq ($(64BIT),yes)
OUT_DIRS = ../../../../../bin/obj/Release_x64/libavcodec_gcc/ \
    ../../../../../bin/obj/Release_x64/libavcodec_gcc/libavcodec \
    ../../../../../bin/obj/Release_x64/libavcodec_gcc/libavcodec/amr_float \
    ../../../../../bin/obj/Release_x64/libavcodec_gcc/libavcodec/x86 \
    ../../../../../bin/obj/Release_x64/libavcodec_gcc/libavutil \
    ../../../../../bin/obj/Release_x64/libavcodec_gcc/libswscale \
    $(SLIB_DIR)
else
OUT_DIRS = ../../../../../bin/obj/Release_Win32/libavcodec_gcc/ \
    ../../../../../bin/obj/Release_Win32/libavcodec_gcc/libavcodec \
    ../../../../../bin/obj/Release_Win32/libavcodec_gcc/libavcodec/amr_float \
    ../../../../../bin/obj/Release_Win32/libavcodec_gcc/libavcodec/x86 \
    ../../../../../bin/obj/Release_Win32/libavcodec_gcc/libavutil \
    ../../../../../bin/obj/Release_Win32/libavcodec_gcc/libswscale \
    $(SLIB_DIR)
endif

CFLAGS+= -I. -I.. -I$(LAVC_DIR) -I$(LAVU_DIR) -I$(LSWS_DIR) -I$(ZLIB_DIR) -I$(PNG_DIR) \
         -DHAVE_AV_CONFIG_H -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -std=gnu99

SRCS_C=\
    $(LAVC_DIR)/aac_ac3_parser.c \
    $(LAVC_DIR)/ac3.c \
    $(LAVC_DIR)/ac3_parser.c \
    $(LAVC_DIR)/ac3dec.c \
    $(LAVC_DIR)/ac3dec_data.c \
    $(LAVC_DIR)/ac3tab.c \
    $(LAVC_DIR)/adpcm.c \
    $(LAVC_DIR)/allcodecs.c \
    $(LAVC_DIR)/avfft.c \
    $(LAVC_DIR)/avpacket.c \
    $(LAVC_DIR)/bitstream.c \
    $(LAVC_DIR)/cabac.c \
    $(LAVC_DIR)/CompilatorVersion.c \
    $(LAVC_DIR)/dsputil.c \
    $(LAVC_DIR)/eac3dec.c \
    $(LAVC_DIR)/eac3dec_data.c \
    $(LAVC_DIR)/error_resilience.c \
    $(LAVC_DIR)/faandct.c \
    $(LAVC_DIR)/faanidct.c \
    $(LAVC_DIR)/fft.c \
    $(LAVC_DIR)/flvdec.c \
    $(LAVC_DIR)/golomb.c \
    $(LAVC_DIR)/h263.c \
    $(LAVC_DIR)/h263_parser.c \
    $(LAVC_DIR)/h263dec.c \
    $(LAVC_DIR)/h264.c \
    $(LAVC_DIR)/h264dsp.c \
    $(LAVC_DIR)/h264_cabac.c \
    $(LAVC_DIR)/h264_cavlc.c \
    $(LAVC_DIR)/h264_direct.c \
    $(LAVC_DIR)/h264_loopfilter.c \
    $(LAVC_DIR)/h264_parser.c \
    $(LAVC_DIR)/h264_ps.c \
    $(LAVC_DIR)/h264_ps.c \
    $(LAVC_DIR)/h264_refs.c \
    $(LAVC_DIR)/h264_sei.c \
    $(LAVC_DIR)/h264idct.c \
    $(LAVC_DIR)/h264pred.c \
    $(LAVC_DIR)/huffman.c \
    $(LAVC_DIR)/imgconvert.c \
    $(LAVC_DIR)/intelh263dec.c \
    $(LAVC_DIR)/intrax8.c \
    $(LAVC_DIR)/intrax8dsp.c \
    $(LAVC_DIR)/ituh263dec.c \
    $(LAVC_DIR)/jpegls.c \
    $(LAVC_DIR)/jpeglsdec.c \
    $(LAVC_DIR)/jrevdct.c \
    $(LAVC_DIR)/libamr.c \
    $(LAVC_DIR)/mdct.c \
    $(LAVC_DIR)/mjpeg.c \
    $(LAVC_DIR)/mjpegdec.c \
    $(LAVC_DIR)/mlp.c \
    $(LAVC_DIR)/mlp_parser.c \
    $(LAVC_DIR)/mlpdec.c \
    $(LAVC_DIR)/mlpdsp.c \
    $(LAVC_DIR)/mpc_helper.c \
    $(LAVC_DIR)/mpeg12.c \
    $(LAVC_DIR)/mpeg12data.c \
    $(LAVC_DIR)/mpeg4video.c \
    $(LAVC_DIR)/mpeg4video_parser.c \
    $(LAVC_DIR)/mpeg4videodec.c \
    $(LAVC_DIR)/mpegvideo.c \
    $(LAVC_DIR)/msmpeg4.c \
    $(LAVC_DIR)/msmpeg4data.c \
    $(LAVC_DIR)/nellymoser.c \
    $(LAVC_DIR)/nellymoserdec.c \
    $(LAVC_DIR)/options.c \
    $(LAVC_DIR)/parser.c \
    $(LAVC_DIR)/rv10.c \
    $(LAVC_DIR)/rv30.c \
    $(LAVC_DIR)/rv30dsp.c \
    $(LAVC_DIR)/rv34.c \
    $(LAVC_DIR)/rv40.c \
    $(LAVC_DIR)/rv40dsp.c \
    $(LAVC_DIR)/simple_idct.c \
    $(LAVC_DIR)/sp5xdec.c \
    $(LAVC_DIR)/svq1.c \
    $(LAVC_DIR)/svq1dec.c \
    $(LAVC_DIR)/svq3.c \
    $(LAVC_DIR)/utils.c \
    $(LAVC_DIR)/vc1.c \
    $(LAVC_DIR)/vc1dec.c \
    $(LAVC_DIR)/vc1data.c \
    $(LAVC_DIR)/vc1dsp.c \
    $(LAVC_DIR)/vp3.c \
    $(LAVC_DIR)/vp3dsp.c \
    $(LAVC_DIR)/vp5.c \
    $(LAVC_DIR)/vp56.c \
    $(LAVC_DIR)/vp56data.c \
    $(LAVC_DIR)/vp56dsp.c \
    $(LAVC_DIR)/vp6.c \
    $(LAVC_DIR)/vp6dsp.c \
    $(LAVC_DIR)/vp8.c \
    $(LAVC_DIR)/vp8dsp.c \
    $(LAVC_DIR)/w32thread.c \
    $(LAVC_DIR)/wmv2.c \
    $(LAVC_DIR)/wmv2dec.c \
    $(LAVC_DIR)/xiph.c \
\
    $(LAVC_DIR)/amr_float/interf_dec.c \
    $(LAVC_DIR)/amr_float/sp_dec.c \
\
    $(LAVC_DIR)/x86/cpuid.c \
    $(LAVC_DIR)/x86/dsputil_mmx.c \
    $(LAVC_DIR)/x86/fdct_mmx.c \
    $(LAVC_DIR)/x86/fft.c \
    $(LAVC_DIR)/x86/fft_3dn.c \
    $(LAVC_DIR)/x86/fft_3dn2.c \
    $(LAVC_DIR)/x86/fft_sse.c \
    $(LAVC_DIR)/x86/idct_mmx.c \
    $(LAVC_DIR)/x86/idct_mmx_xvid.c \
    $(LAVC_DIR)/x86/idct_sse2_xvid.c \
    $(LAVC_DIR)/x86/mlpdsp.c \
    $(LAVC_DIR)/x86/mpegvideo_mmx.c \
    $(LAVC_DIR)/x86/simple_idct_mmx.c \
    $(LAVC_DIR)/x86/vc1dsp_mmx.c \
    $(LAVC_DIR)/x86/vp3dsp_mmx.c \
    $(LAVC_DIR)/x86/vp3dsp_sse2.c \
    $(LAVC_DIR)/x86/vp6dsp_mmx.c \
    $(LAVC_DIR)/x86/vp6dsp_sse2.c \
    $(LAVC_DIR)/x86/vp8dsp-init.c \
\
    $(LAVU_DIR)/crc.c \
    $(LAVU_DIR)/intfloat_readwrite.c \
    $(LAVU_DIR)/inverse.c \
    $(LAVU_DIR)/log.c \
    $(LAVU_DIR)/lfg.c \
    $(LAVU_DIR)/mathematics.c \
    $(LAVU_DIR)/md5.c \
    $(LAVU_DIR)/mem.c \
    $(LAVU_DIR)/pixdesc.c \
    $(LAVU_DIR)/random_seed.c \
    $(LAVU_DIR)/rational.c \
\
    $(LSWS_DIR)/rgb2rgb.c \
    $(LSWS_DIR)/swscale.c \
    $(LSWS_DIR)/sww32thread.c \
    $(LSWS_DIR)/yuv2rgb.c

SRCS_YASM=\
    $(LAVC_DIR)/x86/dsputil_yasm.asm \
    $(LAVC_DIR)/x86/fft_mmx.asm \
    $(LAVC_DIR)/x86/h264_deblock_sse2.asm \
    $(LAVC_DIR)/x86/h264_idct_sse2.asm \
    $(LAVC_DIR)/x86/h264_intrapred.asm \
    $(LAVC_DIR)/x86/vc1dsp_yasm.asm \
    $(LAVC_DIR)/x86/vp8dsp.asm \
    $(LAVC_DIR)/x86/x86util.asm


ifeq ($(64BIT),yes)
    CFLAGS+=-DARCH_X86_64
else
    CFLAGS+=-DARCH_X86_32
endif

SLIB=$(SLIB_DIR)/libavcodec.lib

DEF=libavcodec_gcc.def

NOINTRIN=1

include makefile_c.inc

OPTFLAGS+= -fno-math-errno

.all: outdirs $(SLIB)

.PHONY: outdirs $(OUT_DIRS)
outdirs: $(OUT_DIRS)
$(OUT_DIRS):
	$(shell test -d $(@) || mkdir -p $(@))

.PHONY : clean
clean:
	rm -f $(OUT_DIR)*.o $(OUT_DIR)*.d $(OUT_DIR)*~ \
	$(OUT_DIR)$(LAVC_DIR)/*.o $(OUT_DIR)$(LAVC_DIR)/*.d \
	$(OUT_DIR)$(LAVC_DIR)/x86/*.o $(OUT_DIR)$(LAVC_DIR)/x86/*.d \
	$(OUT_DIR)$(LAVC_DIR)/amr_float/*.o $(OUT_DIR)$(LAVC_DIR)/amr_float/*.d \
	$(OUT_DIR)$(LAVU_DIR)/*.o $(OUT_DIR)$(LAVU_DIR)/*.d \
	$(OUT_DIR)$(LSWS_DIR)/*.o $(OUT_DIR)$(LSWS_DIR)/*.d \
	$(ZLIB_DIR)/*.o $(ZLIB_DIR)/*.d $(PNG_DIR)/*.o $(SLIB)

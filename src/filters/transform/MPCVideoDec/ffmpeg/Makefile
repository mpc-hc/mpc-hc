CC = gcc.exe

BIN_DIR = ../../../../../bin10

ifeq ($(64BIT),yes)
    MY_ARCH = x64
else
    MY_ARCH = Win32
endif

ifeq ($(DEBUG),yes)
    MY_DIR_PREFIX = Debug
else
    MY_DIR_PREFIX = Release
endif


OBJ_DIR        = $(BIN_DIR)/obj/$(MY_DIR_PREFIX)_$(MY_ARCH)/libavcodec_gcc/
TARGET_LIB_DIR = $(BIN_DIR)/lib/$(MY_DIR_PREFIX)_$(MY_ARCH)
TARGET_LIB     = $(TARGET_LIB_DIR)/libavcodec.lib


# Compiler and yasm flags
CFLAGS    = -I. -I.. -Ilibavcodec -Ilibavutil -I../../../../thirdparty/zlib \
            -DHAVE_AV_CONFIG_H -D_ISOC99_SOURCE -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -std=gnu99 -mdll -mthreads -pipe

OPTFLAGS  = -O2 -fno-common -fno-tree-vectorize -fomit-frame-pointer
YASMFLAGS = -I. -Ilibavutil/x86 -Pconfig.asm

ifeq ($(64BIT),yes)
    FFMPEG_PREFIX = x86_64-w64-mingw32-
    TARGET_OS     = x86_64-w64-mingw32
    CFLAGS       += -DWIN64 -D_WIN64 -DARCH_X86_64 -DPIC
    OPTFLAGS     += -m64 -fno-leading-underscore
    YASMFLAGS    += -f win64 -m amd64 -DWIN64 -DARCH_X86_64 -DPIC
else
    TARGET_OS     = i686-pc-mingw32
    CFLAGS       += -DWIN32 -D_WIN32 -DARCH_X86_32
    OPTFLAGS     += -march=i686 -mmmx -msse -mfpmath=sse
    YASMFLAGS    += -f win32 -m x86 -DWIN32 -DARCH_X86_32 -DPREFIX
endif

ifeq ($(DEBUG),yes)
    CFLAGS       += -DDEBUG -D_DEBUG -g
else
    CFLAGS       += -DNDEBUG -UDEBUG
endif


# Object directories
OBJ_DIRS = $(OBJ_DIR) \
    $(OBJ_DIR)libavcodec \
    $(OBJ_DIR)libavcodec/x86 \
    $(OBJ_DIR)libavutil \
    $(OBJ_DIR)libavutil/x86 \
    $(OBJ_DIR)libswscale \
    $(OBJ_DIR)libswscale/x86 \
    $(TARGET_LIB_DIR)


# Targets
all: make_objdirs $(TARGET_LIB)

make_objdirs: $(OBJ_DIRS)
$(OBJ_DIRS):
	$(shell test -d $(@) || mkdir -p $(@))

clean:
	@echo Cleaning...
	rm -f $(TARGET_LIB)
	rm -rf $(OBJ_DIR)


# Objects
SRCS_C=\
    libavcodec/aac_ac3_parser.c \
    libavcodec/ac3.c \
    libavcodec/ac3_parser.c \
    libavcodec/ac3dec.c \
    libavcodec/ac3dec_data.c \
    libavcodec/ac3dsp.c \
    libavcodec/ac3tab.c \
    libavcodec/acelp_filters.c \
    libavcodec/acelp_pitch_delay.c \
    libavcodec/acelp_vectors.c \
    libavcodec/adpcm.c \
    libavcodec/adpcm_data.c \
    libavcodec/allcodecs.c \
    libavcodec/amrnbdec.c \
    libavcodec/amrwbdec.c \
    libavcodec/avfft.c \
    libavcodec/avpacket.c \
    libavcodec/bitstream.c \
    libavcodec/cabac.c \
    libavcodec/celp_filters.c \
    libavcodec/celp_math.c \
    libavcodec/CompilerVersion.c \
    libavcodec/dct.c \
    libavcodec/dct32.c \
    libavcodec/dct32_float.c \
    libavcodec/dsputil.c \
    libavcodec/eac3_data.c \
    libavcodec/eac3dec.c \
    libavcodec/error_resilience.c \
    libavcodec/faandct.c \
    libavcodec/faanidct.c \
    libavcodec/fft.c \
    libavcodec/flvdec.c \
    libavcodec/fmtconvert.c \
    libavcodec/golomb.c \
    libavcodec/h263.c \
    libavcodec/h263_parser.c \
    libavcodec/h263dec.c \
    libavcodec/h264.c \
    libavcodec/h264_cabac.c \
    libavcodec/h264_cavlc.c \
    libavcodec/h264_direct.c \
    libavcodec/h264_loopfilter.c \
    libavcodec/h264_parser.c \
    libavcodec/h264_ps.c \
    libavcodec/h264_refs.c \
    libavcodec/h264_sei.c \
    libavcodec/h264dsp.c \
    libavcodec/h264idct.c \
    libavcodec/h264pred.c \
    libavcodec/huffman.c \
    libavcodec/imgconvert.c \
    libavcodec/intelh263dec.c \
    libavcodec/intrax8.c \
    libavcodec/intrax8dsp.c \
    libavcodec/ituh263dec.c \
    libavcodec/jpegls.c \
    libavcodec/jpeglsdec.c \
    libavcodec/jrevdct.c \
    libavcodec/kbdwin.c \
    libavcodec/lsp.c \
    libavcodec/mdct.c \
    libavcodec/mjpeg.c \
    libavcodec/mjpegdec.c \
    libavcodec/mlp.c \
    libavcodec/mlp_parser.c \
    libavcodec/mlpdec.c \
    libavcodec/mlpdsp.c \
    libavcodec/mpc_helper.c \
    libavcodec/mpeg12.c \
    libavcodec/mpeg12data.c \
    libavcodec/mpeg4video.c \
    libavcodec/mpeg4video_parser.c \
    libavcodec/mpeg4videodec.c \
    libavcodec/mpegaudio.c \
    libavcodec/mpegaudio_parser.c \
    libavcodec/mpegaudiodata.c \
    libavcodec/mpegaudiodec.c \
    libavcodec/mpegaudiodec_float.c \
    libavcodec/mpegaudiodecheader.c \
    libavcodec/mpegaudiodsp.c \
    libavcodec/mpegaudiodsp_float.c \
    libavcodec/mpegaudiodsp_template.c \
    libavcodec/mpegvideo.c \
    libavcodec/msmpeg4.c \
    libavcodec/msmpeg4data.c \
    libavcodec/nellymoser.c \
    libavcodec/nellymoserdec.c \
    libavcodec/options.c \
    libavcodec/parser.c \
    libavcodec/pthread.c \
    libavcodec/rdft.c \
    libavcodec/rv10.c \
    libavcodec/rv30.c \
    libavcodec/rv30dsp.c \
    libavcodec/rv34.c \
    libavcodec/rv34dsp.c \
    libavcodec/rv40.c \
    libavcodec/rv40dsp.c \
    libavcodec/simple_idct.c \
    libavcodec/sinewin.c \
    libavcodec/sp5xdec.c \
    libavcodec/svq1.c \
    libavcodec/svq1dec.c \
    libavcodec/svq3.c \
    libavcodec/utils.c \
    libavcodec/vc1.c \
    libavcodec/vc1dec.c \
    libavcodec/vc1data.c \
    libavcodec/vc1dsp.c \
    libavcodec/vp3.c \
    libavcodec/vp3dsp.c \
    libavcodec/vp5.c \
    libavcodec/vp56.c \
    libavcodec/vp56data.c \
    libavcodec/vp56dsp.c \
    libavcodec/vp56rac.c \
    libavcodec/vp6.c \
    libavcodec/vp6dsp.c \
    libavcodec/vp8.c \
    libavcodec/vp8dsp.c \
    libavcodec/wmv2.c \
    libavcodec/wmv2dec.c \
    libavcodec/xiph.c \
    libavcodec/x86/ac3dsp_mmx.c \
    libavcodec/x86/dsputil_mmx.c \
    libavcodec/x86/fdct_mmx.c \
    libavcodec/x86/fft.c \
    libavcodec/x86/fft_3dn.c \
    libavcodec/x86/fft_3dn2.c \
    libavcodec/x86/fft_sse.c \
    libavcodec/x86/fmtconvert_mmx.c \
    libavcodec/x86/h264_intrapred_init.c \
    libavcodec/x86/h264dsp_mmx.c \
    libavcodec/x86/idct_mmx.c \
    libavcodec/x86/idct_mmx_xvid.c \
    libavcodec/x86/idct_sse2_xvid.c \
    libavcodec/x86/mlpdsp.c \
    libavcodec/x86/motion_est_mmx.c \
    libavcodec/x86/mpegaudiodec_mmx.c \
    libavcodec/x86/mpegvideo_mmx.c \
    libavcodec/x86/rv40dsp.c \
    libavcodec/x86/simple_idct_mmx.c \
    libavcodec/x86/vc1dsp_mmx.c \
    libavcodec/x86/vp56dsp_init.c \
    libavcodec/x86/vp8dsp-init.c \
\
    libavutil/audioconvert.c \
    libavutil/avstring.c \
    libavutil/cpu.c \
    libavutil/crc.c \
    libavutil/dict.c \
    libavutil/eval.c \
    libavutil/imgutils.c \
    libavutil/intfloat_readwrite.c \
    libavutil/inverse.c \
    libavutil/log.c \
    libavutil/lfg.c \
    libavutil/mathematics.c \
    libavutil/md5.c \
    libavutil/mem.c \
    libavutil/opt.c \
    libavutil/pixdesc.c \
    libavutil/random_seed.c \
    libavutil/rational.c \
    libavutil/samplefmt.c \
    libavutil/utils.c \
    libavutil/x86/cpu.c \
\
    libswscale/rgb2rgb.c \
    libswscale/swscale.c \
    libswscale/swscale_unscaled.c \
    libswscale/utils.c \
    libswscale/sww32thread.c \
    libswscale/yuv2rgb.c \
    libswscale/x86/rgb2rgb.c \
    libswscale/x86/swscale_mmx.c \
    libswscale/x86/yuv2rgb_mmx.c



# Yasm objects
SRCS_YASM=\
    libavcodec/x86/ac3dsp.asm \
    libavcodec/x86/dsputil_yasm.asm \
    libavcodec/x86/dct32_sse.asm \
    libavcodec/x86/fft_mmx.asm \
    libavcodec/x86/fmtconvert.asm \
    libavcodec/x86/h264_chromamc.asm \
    libavcodec/x86/h264_chromamc_10bit.asm \
    libavcodec/x86/h264_deblock.asm \
    libavcodec/x86/h264_deblock_10bit.asm \
    libavcodec/x86/h264_idct.asm \
    libavcodec/x86/h264_idct_10bit.asm \
    libavcodec/x86/h264_intrapred.asm \
    libavcodec/x86/h264_intrapred_10bit.asm \
    libavcodec/x86/h264_qpel_10bit.asm \
    libavcodec/x86/h264_weight.asm \
    libavcodec/x86/h264_weight_10bit.asm \
    libavcodec/x86/vc1dsp_yasm.asm \
    libavcodec/x86/vp3dsp.asm \
    libavcodec/x86/vp56dsp.asm \
    libavcodec/x86/vp8dsp.asm \
    libswscale/x86/scale.asm

OBJS += \
    $(SRCS_C:%.c=$(OBJ_DIR)%.o) \
    $(SRCS_YASM:%.asm=$(OBJ_DIR)%.o)


# Commands
$(OBJ_DIR)%.o: %.c
	@echo CC $<
	@$(FFMPEG_PREFIX)$(CC) -c $(CFLAGS) $(OPTFLAGS) -MMD -o $@ $<

$(OBJ_DIR)%.o: %.asm
	@echo YASM: Assembling $<
	@yasm $(YASMFLAGS) -I$(<D)/ -o $@ $<

$(TARGET_LIB): $(OBJS)
	@echo AR $@
	@$(FFMPEG_PREFIX)ar rc $@ $(OBJS)


.PHONY: clean make_objdirs $(OBJ_DIRS)

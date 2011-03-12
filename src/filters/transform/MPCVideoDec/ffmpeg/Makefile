ifeq ($(VS2010),yes)
    BIN_DIR = ../../../../../bin10
else
    BIN_DIR = ../../../../../bin
endif

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
CFLAGS    = -I. -I.. -Ilibavcodec -Ilibavcore -Ilibavutil -Ilibswscale \
            -DHAVE_AV_CONFIG_H -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 \
            -std=gnu99 -mno-cygwin -mdll -mthreads -pipe

OPTFLAGS  = -fno-common -fno-tree-vectorize
YASMFLAGS = -Pconfig.asm

ifeq ($(64BIT),yes)
    FFMPEG_PREFIX = x86_64-w64-mingw32-
    TARGET_OS     = x86_64-w64-mingw32
    CFLAGS       += -DWIN64 -D_WIN64 -DARCH_X86_64
    OPTFLAGS     += -O2 -m64 -fno-leading-underscore
    YASMFLAGS    += -f win64 -m amd64 -DWIN64 -DARCH_X86_64
else
    TARGET_OS     = i686-pc-mingw32
    CFLAGS       += -DWIN32 -D_WIN32 -DARCH_X86_32
    ifeq ($(VS2010),yes)
        OPTFLAGS += -O2 -march=i686 -mmmx -msse -msse2 -mfpmath=sse
    else
       OPTFLAGS  += -O2 -march=i686 -mmmx -msse -mfpmath=sse
    endif
    YASMFLAGS    += -f win32 -m x86 -DWIN32 -DARCH_X86_32 -DPREFIX
endif

ifeq ($(DEBUG),yes)
    CFLAGS       += -DDEBUG -D_DEBUG -g
    YASMFLAGS    += -g cv8
else
    CFLAGS       += -DNDEBUG -UDEBUG
    OPTFLAGS     += -fomit-frame-pointer
endif


# Object directories
OBJ_DIRS = $(OBJ_DIR) \
    $(OBJ_DIR)libavcodec \
    $(OBJ_DIR)libavcodec/amr_float \
    $(OBJ_DIR)libavcodec/x86 \
    $(OBJ_DIR)libavcore \
    $(OBJ_DIR)libavutil \
    $(OBJ_DIR)libavutil/x86 \
    $(OBJ_DIR)libswscale \
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
    libavcodec/ac3tab.c \
    libavcodec/adpcm.c \
    libavcodec/allcodecs.c \
    libavcodec/avfft.c \
    libavcodec/avpacket.c \
    libavcodec/bitstream.c \
    libavcodec/cabac.c \
    libavcodec/CompilerVersion.c \
    libavcodec/dsputil.c \
    libavcodec/eac3dec.c \
    libavcodec/eac3dec_data.c \
    libavcodec/error_resilience.c \
    libavcodec/faandct.c \
    libavcodec/faanidct.c \
    libavcodec/fft.c \
    libavcodec/flvdec.c \
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
    libavcodec/libamr.c \
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
    libavcodec/mpegvideo.c \
    libavcodec/msmpeg4.c \
    libavcodec/msmpeg4data.c \
    libavcodec/nellymoser.c \
    libavcodec/nellymoserdec.c \
    libavcodec/options.c \
    libavcodec/parser.c \
    libavcodec/rv10.c \
    libavcodec/rv30.c \
    libavcodec/rv30dsp.c \
    libavcodec/rv34.c \
    libavcodec/rv40.c \
    libavcodec/rv40dsp.c \
    libavcodec/simple_idct.c \
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
    libavcodec/w32thread.c \
    libavcodec/wmv2.c \
    libavcodec/wmv2dec.c \
    libavcodec/xiph.c \
\
    libavcodec/amr_float/interf_dec.c \
    libavcodec/amr_float/sp_dec.c \
\
    libavcodec/x86/dsputil_mmx.c \
    libavcodec/x86/fdct_mmx.c \
    libavcodec/x86/fft.c \
    libavcodec/x86/fft_3dn.c \
    libavcodec/x86/fft_3dn2.c \
    libavcodec/x86/fft_sse.c \
    libavcodec/x86/h264_intrapred_init.c \
    libavcodec/x86/h264dsp_mmx.c \
    libavcodec/x86/idct_mmx.c \
    libavcodec/x86/idct_mmx_xvid.c \
    libavcodec/x86/idct_sse2_xvid.c \
    libavcodec/x86/mlpdsp.c \
    libavcodec/x86/mpegvideo_mmx.c \
    libavcodec/x86/simple_idct_mmx.c \
    libavcodec/x86/vc1dsp_mmx.c \
    libavcodec/x86/vp56dsp_init.c \
    libavcodec/x86/vp8dsp-init.c \
\
    libavcore/audioconvert.c \
    libavcore/avcore_utils.c \
    libavcore/imgutils.c \
    libavcore/samplefmt.c \
\
    libavutil/avstring.c \
    libavutil/cpu.c \
    libavutil/crc.c \
    libavutil/eval.c \
    libavutil/intfloat_readwrite.c \
    libavutil/inverse.c \
    libavutil/log.c \
    libavutil/lfg.c \
    libavutil/mathematics.c \
    libavutil/md5.c \
    libavutil/mem.c \
    libavutil/pixdesc.c \
    libavutil/random_seed.c \
    libavutil/rational.c \
\
    libavutil/x86/cpu.c \
\
    libswscale/rgb2rgb.c \
    libswscale/swscale.c \
    libswscale/sww32thread.c \
    libswscale/yuv2rgb.c

# Yasm objects
SRCS_YASM=\
    libavcodec/x86/dsputil_yasm.asm \
    libavcodec/x86/fft_mmx.asm \
    libavcodec/x86/h264_chromamc.asm \
    libavcodec/x86/h264_deblock.asm \
    libavcodec/x86/h264_idct.asm \
    libavcodec/x86/h264_intrapred.asm \
    libavcodec/x86/h264_weight.asm \
    libavcodec/x86/vc1dsp_yasm.asm \
    libavcodec/x86/vp3dsp.asm \
    libavcodec/x86/vp56dsp.asm \
    libavcodec/x86/vp8dsp.asm \
    libavcodec/x86/x86util.asm


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

$(TARGET_LIB): $(OBJS) libavcodec.def
	@echo AR $@
	@$(FFMPEG_PREFIX)ar rc $@ $(OBJS)


.PHONY: clean make_objdirs $(OBJ_DIRS)

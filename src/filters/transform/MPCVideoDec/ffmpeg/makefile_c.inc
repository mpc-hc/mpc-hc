all:.all
distclean: clean

CFLAGS= -I. -I.. -Ilibavcodec -Ilibavcore -Ilibavutil -Ilibswscale -I$(ZLIB_DIR) \
         -I$(PNG_DIR) -DHAVE_AV_CONFIG_H -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 \
         -std=gnu99 -mno-cygwin -mdll -mthreads -pipe

OPTFLAGS=-fno-common -fno-tree-vectorize
YASMFLAGS=-Pconfig.asm

ifeq ($(64BIT),yes)
    FFMPEG_PREFIX=x86_64-w64-mingw32-
    TARGET_OS=x86_64-w64-mingw32
    CFLAGS+=-DWIN64 -D_WIN64 -DARCH_X86_64
    OPTFLAGS+=-O2 -m64 -fno-leading-underscore
    YASMFLAGS+=-f win64 -m amd64 -DWIN64 -DARCH_X86_64
else
    TARGET_OS=i686-pc-mingw32
    CFLAGS+=-DWIN32 -D_WIN32 -DARCH_X86_32
    OPTFLAGS+=-O2 -march=i686 -mmmx -msse -mfpmath=sse
    YASMFLAGS+=-f win32 -m x86 -DWIN32 -DARCH_X86_32 -DPREFIX
endif

ifeq ($(DEBUG),yes)
    CFLAGS+=-DDEBUG -D_DEBUG -g 
    YASMFLAGS+=-g cv8
else
    CFLAGS+=-DNDEBUG -UDEBUG
    OPTFLAGS+=-fomit-frame-pointer
endif


OBJS+=$(SRCS_C:%.c=$(OBJ_DIR)%.o) $(SRCS_YASM:%.asm=$(OBJ_DIR)%.o)

$(OBJ_DIR)%.o: %.c
	@echo CC $<
	@$(FFMPEG_PREFIX)$(CC) -c $(CFLAGS) $(OPTFLAGS) -MMD -o $@ $<

$(OBJ_DIR)%.o: %.asm
	@echo YASM: Assembling $<
	@yasm $(YASMFLAGS) -I$(<D)/ -o $@ $<

$(TARGET_LIB): $(OBJS) libavcodec.def
	@echo AR $@
	@$(FFMPEG_PREFIX)ar rc $@ $(OBJS)

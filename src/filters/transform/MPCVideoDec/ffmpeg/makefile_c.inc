all:.all

#64BIT=yes
#DEBUG=yes

#if all FFMPEG_PREFIX is set, assume cross-compilation
ifeq ($(FFMPEG_PREFIX),)
  YASM=yasm.exe
else
  YASM=yasm
endif

ifeq ($(64BIT),yes)
    FFMPEG_PREFIX=x86_64-w64-mingw32-
endif

ifeq ($(64BIT),yes)
    TARGET_OS=x86_64-w64-mingw32
    CFLAGS+=-DWIN64 -D_WIN64
    OPTFLAGS+=-O2 -m64 -fno-leading-underscore
else
    TARGET_OS=i686-pc-mingw32
    CFLAGS+=-DWIN32 -D_WIN32
    OPTFLAGS+=-O2 -march=i686 -mmmx -msse -mfpmath=sse
endif

CFLAGS+=-mno-cygwin -mdll -mthreads -pipe
OPTFLAGS+=-fno-common -fno-tree-vectorize

ifeq ($(DEBUG),yes)
    CFLAGS+=-DDEBUG -D_DEBUG -g 
    YASMFLAGS+=-g cv8
else
    CFLAGS+=-DNDEBUG -UDEBUG
    OPTFLAGS+=-fomit-frame-pointer
    LDFLAGS+=--strip-all
endif

ifeq ($(UNICODE),yes)
    CFLAGS+=-DUNICODE -D_UNICODE
endif

ifeq ($(FORCECPP),1)
    CFLAGS+=-x c++
endif

CPPFLAGS+=-fno-rtti

ifeq ($(EXCEPTIONS),1)
    CPPFLAGS+=-fexceptions
else
    CPPFLAGS+=-fno-exceptions
endif

OUTPUTFLAG=-MMD -o\

GCCDEF=$(DEF)

OBJS+=$(SRCS:%.cpp=$(OUT_DIR)%.o) $(SRCS_C:%.c=$(OUT_DIR)%.o) $(SRCS_YASM:%.asm=$(OUT_DIR)%.o) $(RESSRC:%.rc=$(OUT_DIR)%.res.o)

$(OUT_DIR)%.o: %.c
	@echo $<
	@$(FFMPEG_PREFIX)$(CC) -c $(CFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<

$(OUT_DIR)%.o: %.cpp
	$(FFMPEG_PREFIX)$(CC) -c $(CFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<

$(OUT_DIR)%.o: %.asm
ifeq ($(64BIT),yes)
	$(YASM) -f win64 -m amd64 -DWIN64 -DARCH_X86_64 $(YASMFLAGS) -I$(<D)/ -o $@ $<
else
	$(YASM) -f win32 -m x86 -DWIN32 -DARCH_X86_32 -DPREFIX $(YASMFLAGS) -I$(<D)/ -o $@ $<
endif

$(OUT_DIR)%.res.o: %.rc
	$(FFMPEG_PREFIX)windres $< -o $@ -D_WIN32_IE=0x0500 -DIDCLOSE=8

$(SLIB): $(OBJS) $(DEF)
	$(FFMPEG_PREFIX)ar rc $@ $(OBJS)
#===> Modify for MPC-HC to create library instead of DLL
#	$(FFMPEG_PREFIX)dllwrap -mno-cygwin --target=$(TARGET_OS) \
#             -Wl,--enable-stdcall-fixup,--strip-all \
#             --def $(GCCDEF) $(GCCDLLENTRY) -o $@ $(OBJS) $(GCCLIBS)
#<=== Modify for MPC-HC to create library instead of DLL

$(SEXE): $(OBJS)
	$(FFMPEG_PREFIX)$(CC) -mno-cygwin --target=$(TARGET_OS) \
             -Wl,--enable-stdcall-fixup -o $@ $(OBJS) $(GCCLIBS)
	$(FFMPEG_PREFIX)strip $@

distclean: clean

TEMPFILES=*.i *.ii *.d *.s *.o

-include $(SRCS:.cpp=.d) $(SRCS_C:.c=.d)

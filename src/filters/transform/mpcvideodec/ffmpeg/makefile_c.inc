ifeq ($(FFMPEG_PREFIX),)
    YASM=yasm.exe
else
    YASM=yasm
endif

ifeq ($(64BIT),yes)
FFMPEG_PREFIX=x86_64-pc-mingw32-
endif

ifeq ($(64BIT),yes)
	TARGET_OS=x86_64-pc-mingw32
else
	TARGET_OS=i686-pc-mingw32
endif

all:.all

OUTPUTFLAG=-MMD -o\

CFLAGS+=-mno-cygwin -mdll -mthreads -pipe
CFLAGS+=-DNDEBUG -UDEBUG

ifeq ($(64BIT),yes)
	CFLAGS+=-DWIN64 -D_WIN64
else
	CFLAGS+=-DWIN32 -D_WIN32
endif

ifeq ($(64BIT),yes)
	CFLAGS+=-msse2
else
	CFLAGS+=-mmmx
endif

ifeq ($(SSE),yes)
	CFLAGS+=-msse -mfpmath=sse
endif
ifeq ($(SSE2),yes)
	CFLAGS+=-msse2
endif

OPTFLAGS=-O3 -fomit-frame-pointer
ifeq ($(64BIT),yes)
	OPTFLAGS+=-m64 -fno-leading-underscore
else
	OPTFLAGS+=-march=i686
endif

CPPFLAGS+=-fno-rtti

GCCDEF=$(DEF)

ifeq ($(UNICODE),yes)
	CFLAGS+=-DUNICODE -D_UNICODE
endif

OBJS+=$(SRCS:.cpp=.o) $(SRCS_C:.c=.o) $(SRCS_YASM:.asm=.o) $(RESSRC:.rc=.res.o)

ifeq ($(FORCECPP),1)
	CFLAGS+=-x c++
endif

ifeq ($(EXCEPTIONS),1)
	CPPFLAGS+=-fexceptions
else
	CPPFLAGS+=-fno-exceptions
endif

.c.o:
	$(FFMPEG_PREFIX)$(CC) -c $(CFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<

.cpp.o:
	$(FFMPEG_PREFIX)$(CC) -c $(CFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<

%.o: %.asm
ifeq ($(64BIT),yes)
	$(YASM) -f win64 -DWIN64 -DARCH_X86_64 -DPREFIX $(YASMFLAGS) -I$(<D)/ -o $@  $<
else
	$(YASM) -f win32 -DWIN32 -DARCH_X86_32 -DPREFIX $(YASMFLAGS) -I$(<D)/ -o $@  $<
endif

%.res.o: %.rc
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

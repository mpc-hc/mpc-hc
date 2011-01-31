all:.all
distclean: clean

ifeq ($(64BIT),yes)
    FFMPEG_PREFIX=x86_64-w64-mingw32-
    TARGET_OS=x86_64-w64-mingw32
    CFLAGS+=-DWIN64 -D_WIN64
    OPTFLAGS+=-O2 -m64
    YASMFLAGS+=-f win64 -m amd64 -DWIN64 -DARCH_X86_64
else
    TARGET_OS=i686-pc-mingw32
    CFLAGS+=-DWIN32 -D_WIN32
    OPTFLAGS+=-O2 -march=i686 -mmmx -msse -mfpmath=sse
    YASMFLAGS+=-f win32 -m x86 -DWIN32 -DARCH_X86_32 -DPREFIX
endif

CFLAGS+=-mno-cygwin -mdll -mthreads -pipe
OPTFLAGS+=-fno-common -fno-tree-vectorize
CPPFLAGS+=-fno-rtti
OUTPUTFLAG=-MMD -o\

ifeq ($(DEBUG),yes)
    CFLAGS+=-DDEBUG -D_DEBUG -g 
    YASMFLAGS+=-g cv8
else
    CFLAGS+=-DNDEBUG -UDEBUG
    OPTFLAGS+=-fomit-frame-pointer
    LDFLAGS+=--strip-all
endif

ifeq ($(EXCEPTIONS),1)
    CPPFLAGS+=-fexceptions
else
    CPPFLAGS+=-fno-exceptions
endif


OBJS+=$(SRCS:%.cpp=$(OBJ_DIR)%.o) $(SRCS_C:%.c=$(OBJ_DIR)%.o) $(SRCS_YASM:%.asm=$(OBJ_DIR)%.o) $(RESSRC:%.rc=$(OBJ_DIR)%.res.o)

$(OBJ_DIR)%.o: %.c
	@echo $<
	@$(FFMPEG_PREFIX)$(CC) -c $(CFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<

$(OBJ_DIR)%.o: %.cpp
	@echo $<
	@$(FFMPEG_PREFIX)$(CC) -c $(CFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<

$(OBJ_DIR)%.o: %.asm
	@echo $<
	@yasm $(YASMFLAGS) -I$(<D)/ -o $@ $<

$(OBJ_DIR)%.res.o: %.rc
	@echo $<
	@$(FFMPEG_PREFIX)windres $< -o $@ -D_WIN32_IE=0x0500 -DIDCLOSE=8

$(TARGET_LIB): $(OBJS) $(DEF)
	@echo $@
	@$(FFMPEG_PREFIX)ar rc $@ $(OBJS)

$(SEXE): $(OBJS)
	@echo $@
	@$(FFMPEG_PREFIX)$(CC) -mno-cygwin --target=$(TARGET_OS) \
			-Wl,--enable-stdcall-fixup -o $@ $(OBJS) $(GCCLIBS)
	@$(FFMPEG_PREFIX)strip $@

-include $(SRCS:%.cpp=$(OBJ_DIR)%.d) $(SRCS_C:%.c=$(OBJ_DIR)%.d)

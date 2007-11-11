all:.all

#if FFDSHOW_PREFIX is set, assume cross-compilation
ifeq ($(FFDSHOW_PREFIX),)
  NASM=yasm.exe
else
  NASM=yasm
endif

ifeq ($(CC),icl)
  OUTPUTFLAG=/Fo

  CFLAGS+=/nologo /MD /LD
  CFLAGS+=/D "WIN64" /D "_WIN64" /D "_WINDOWS" /D "_USERDLL" /D "NDEBUG"
  OPTFLAGS=/O3 /G7 /GF /Qip /Zc:wchar_t
else
ifeq ($(CC),cl)
  OUTPUTFLAG=/Fo

  CFLAGS+=/nologo /MD /LD
  CFLAGS+=/D "WIN32" /D "_WIN32" /D "_WINDOWS" /D "_USERDLL" /D "NDEBUG"
  OPTFLAGS=/Ox /G7 /Zc:wchar_t
  ifeq ($(SSE),yes)
    OPTFLAGS+=/arch:SSE
  endif
else
  OUTPUTFLAG=-MMD -o\

  CFLAGS+=-mno-cygwin -mdll -mthreads -pipe
  CFLAGS+=-mmmx
  ifeq ($(SSE),yes)
    CFLAGS+=-msse -mfpmath=sse
  endif
  ifeq ($(SSE2),yes)
    CFLAGS+=-msse2
  endif
  CFLAGS+=-DNDEBUG -UDEBUG -DWIN32 -D_WIN32
  OPTFLAGS=-O2 -march=pentium-mmx -mtune=i686 -fomit-frame-pointer -finline-functions -finline -frename-registers -fweb -funit-at-a-time
  CPPFLAGS+=-fno-rtti

  GCCDEF=$(DEF)
endif
endif

ifeq ($(UNICODE),yes)
  CFLAGS+=-DUNICODE -D_UNICODE
endif

OBJS+=$(SRCS:.cpp=.o) $(SRCS_C:.c=.o) $(SRCS_NASM:.asm=.o) $(RESSRC:.rc=.res.o)

ifeq ($(FORCECPP),1)
  ifeq ($(CC),icl)
    CFLAGS+=/TP
  else
  ifeq ($(CC),cl)
    CFLAGS+=/TP
  else
    CFLAGS+=-x c++
  endif
  endif
endif

ifeq ($(EXCEPTIONS),1)
  ifeq ($(CC),icl)
    CPPFLAGS+=/EHsc
  else
  ifeq ($(CC),cl)
    CPPFLAGS+=/EHsc
  else
    CPPFLAGS+=-fexceptions
  endif
  endif
else
  ifeq ($(CC),icl)
  else
  ifeq ($(CC),cl)
  else
    CPPFLAGS+=-fno-exceptions
  endif
  endif
endif

.c.o:
	$(FFDSHOW_PREFIX)$(CC) -c $(CFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<

.cpp.o:
	$(FFDSHOW_PREFIX)$(CC) -c $(CFLAGS) $(CPPFLAGS) $(OPTFLAGS) $(OUTPUTFLAG)$@ $<

%.o: %.asm
	$(NASM) -f win32 -DWIN32 -DPREFIX $(NASMFLAGS) -I$(<D)/ -o $@  $<

ifeq ($(CC),icl)
%.res.o: %.rc
	rc /Fo$@ $<
else
ifeq ($(CC),cl)
%.res.o: %.rc
	rc /Fo$@ $<
else
%.res.o: %.rc
	$(FFDSHOW_PREFIX)windres $< -o $@ -D_WIN32_IE=0x0500 -DIDCLOSE=8
endif
endif

ifeq ($(CC),icl)
$(SLIB):$(OBJS) $(DEF)
	link $(OBJS) msvcrt.lib kernel32.lib oldnames.lib libircmt.lib libmmds.lib $(CLLIBS) \
             /dll /machine:AMD64 /nodefaultlib \
             /def:"$(DEF)" \
             $(CLDLLENTRY) \
             /out:$(SLIB)
else
ifeq ($(CC),cl)
$(SLIB):$(OBJS) $(DEF)
	link $(OBJS) msvcrt.lib kernel32.lib oldnames.lib $(CLLIBS) \
             /dll /machine:I386 /nodefaultlib \
             /def:"$(DEF)" \
             $(CLDLLENTRY) \
             /out:$(SLIB)
else
$(SLIB): $(OBJS) $(DEF)
#	=== Modify for MPC-HC to create library instead of DLL
#	$(FFDSHOW_PREFIX)dllwrap -mno-cygwin --target=i386-mingw32 \
#             -Wl,--enable-stdcall-fixup,--strip-all \
#             --def $(GCCDEF) $(GCCDLLENTRY) -o $@ $(OBJS) $(GCCLIBS)
	$(FFDSHOW_PREFIX)ar rc $@ $(OBJS) 
endif
endif

ifeq ($(CC),icl)
$(SEXE): $(OBJS) $(DEF)
	link $(OBJS) msvcrt.lib kernel32.lib oldnames.lib libircmt.lib libmmds.lib $(CLLIBS) $(ICLLIBS) \
             /machine:AMD64 /nodefaultlib \
             /out:$(SEXE)
else
ifeq ($(CC),cl)
$(SEXE): $(OBJS) $(DEF)
	link $(OBJS) msvcrt.lib kernel32.lib oldnames.lib $(CLLIBS) \
             /machine:I386 /nodefaultlib \
             /out:$(SEXE)
else
$(SEXE): $(OBJS)
	$(FFDSHOW_PREFIX)$(CC) -mno-cygwin --target=i386-mingw32 \
             -Wl,--enable-stdcall-fixup -o $@ $(OBJS) $(GCCLIBS)
	$(FFDSHOW_PREFIX)strip $@
endif
endif

distclean: clean

TEMPFILES=*.i *.ii *.d *.s *.o

-include $(SRCS:.cpp=.d) $(SRCS_C:.c=.d)

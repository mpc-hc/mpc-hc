# $Id: Makefile.am 280 2005-01-30 05:11:46Z gabest $
#

INCLUDES = -I$(top_srcdir) -I$(srcdir)

h_sources = arith_codec.h arrays.h band_codec.h bit_manager.h \
            common_types.h common.h frame.h golomb.h frame_buffer.h \
            mot_comp.h motion.h mv_codec.h pic_io.h upconvert.h \
            wavelet_utils.h cmd_line.h dirac_assertions.h dirac_types.h

cpp_sources = band_codec.cpp bit_manager.cpp common.cpp \
              frame.cpp golomb.cpp frame_buffer.cpp mot_comp.cpp motion.cpp \
              mv_codec.cpp pic_io.cpp upconvert.cpp wavelet_utils.cpp \
              cmd_line.cpp dirac_assertions.cpp


if USE_MSVC
noinst_LIBRARIES = libdirac_common.a
libdirac_common_a_AR = link.exe -lib /out:libdirac_common.a $(libdirac_common_a_OBJECTS) ; echo
libdirac_common_a_SOURCES = $(h_sources) $(cpp_sources)
else
libdirac_common_a_AR = ar crv
noinst_LTLIBRARIES = libdirac_common.la
libdirac_common_la_SOURCES = $(h_sources) $(cpp_sources)
libdirac_common_la_CXXFLAGS = $(AM_CXXFLAGS)
endif



library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)/libdirac_common
library_include_HEADERS = common_types.h dirac_types.h 

if USE_MSVC
CLEANFILES = *.pdb *.ilk
endif


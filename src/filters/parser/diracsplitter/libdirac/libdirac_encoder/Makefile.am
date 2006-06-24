# $Id: Makefile.am 280 2005-01-30 05:11:46Z gabest $
#

INCLUDES = -I$(top_srcdir) -I$(srcdir)

h_sources = comp_compress.h frame_compress.h quality_monitor.h \
            seq_compress.h dirac_encoder.h

cpp_sources = comp_compress.cpp frame_compress.cpp quality_monitor.cpp \
              seq_compress.cpp dirac_encoder.cpp


if USE_MSVC
lib_LIBRARIES = libdirac_encoder.a
libdirac_encoder_a_AR = link -lib -out:libdirac_encoder.a $(libdirac_encoder_a_OBJECTS); link $(LIBFLAGS) -out:libdirac_encoder$(LIBEXT) $(LDFLAGS) $(libdirac_encoder_a_OBJECTS) $(libdirac_encoder_a_DEPENDENCIES); echo
else
libdirac_encoder_a_AR = ar crv
lib_LTLIBRARIES = libdirac_encoder.la
endif

libdirac_encoder_a_SOURCES = $(h_sources) $(cpp_sources)

libdirac_encoder_a_LIBADD = \
               $(top_builddir)/libdirac_motionest/libdirac_motionest.a \
               $(top_builddir)/libdirac_common/libdirac_common.a

libdirac_encoder_la_SOURCES = $(h_sources) $(cpp_sources)
libdirac_encoder_la_CXXFLAGS = $(AM_CXXFLAGS)
libdirac_encoder_la_LIBADD = \
         $(top_builddir)/libdirac_motionest/libdirac_motionest.la \
         $(top_builddir)/libdirac_common/libdirac_common.la

libdirac_encoder_la_LDFLAGS= -no-undefined -version-info $(GENERIC_LIBRARY_VERSION)

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)/libdirac_encoder
library_include_HEADERS = dirac_encoder.h

#Custom install rule for MSCV
if USE_MSVC
install-exec-local: libdirac_encoder.lib
	    $(mkinstalldirs) "$(DESTDIR)$(libdir)"; \
	    echo " $(INSTALL_PROGRAM) libdirac_encoder.lib $(DESTDIR)$(libdir)/libdirac_encoder.lib"; \
	    $(INSTALL_PROGRAM) libdirac_encoder.lib $(DESTDIR)$(libdir)/libdirac_encoder.lib || exit 1; \
	for ext in lib dll pdb; do \
	  file="libdirac_encoder.$$ext"; \
	  if test -f "$$file"; then \
	    $(mkinstalldirs) "$(DESTDIR)$(bindir)"; \
	    echo " $(INSTALL_PROGRAM) $$file $(DESTDIR)$(bindir)/$$file"; \
	    $(INSTALL_PROGRAM) "$$file" "$(DESTDIR)$(bindir)/$$file" || exit 1; \
	  fi \
	done

uninstall-local:
	for file in \
	  "$(DESTDIR)$(bindir)/libdirac_encoder.dll" \
	  "$(DESTDIR)$(bindir)/libdirac_encoder.pdb" \
	  "$(DESTDIR)$(libdir)/libdirac_encoder.lib"; \
	  do \
	  if test -f "$$file"; then \
	    echo " rm -f" "$$file"; \
	    rm -f "$$file"; \
	  fi \
	done
endif

if USE_MSVC
CLEANFILES = *.pdb *.ilk *.dll *.lib *.exp *.a
endif


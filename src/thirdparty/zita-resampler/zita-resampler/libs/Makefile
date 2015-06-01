# ----------------------------------------------------------------------------
#
#  Copyright (C) 2006-2012 Fons Adriaensen <fons@linuxaudio.org>
#    
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# ----------------------------------------------------------------------------


# Modify as required.
#
PREFIX = /usr/local
SUFFIX := $(shell uname -m | sed -e 's/^unknown/$//' -e 's/^i.86/$//' -e 's/^x86_64/$/64/')
LIBDIR = lib$(SUFFIX)

MAJVERS = 1
MINVERS = 3.0
VERSION = $(MAJVERS).$(MINVERS)
DISTDIR = zita-resampler-$(VERSION)


CPPFLAGS += -I. -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS
CXXFLAGS += -Wall -fPIC -O2 -ffast-math
CXXFLAGS += -march=native
LDFLAGS += 
LDLIBS +=


ZITA-RESAMPLER_SO = libzita-resampler.so
ZITA-RESAMPLER_MAJ = $(ZITA-RESAMPLER_SO).$(MAJVERS)
ZITA-RESAMPLER_MIN = $(ZITA-RESAMPLER_MAJ).$(MINVERS)
ZITA-RESAMPLER_DEP =
ZITA-RESAMPLER_O = resampler.o vresampler.o resampler-table.o
ZITA-RESAMPLER_H = zita-resampler/resampler.h zita-resampler/resampler-table.h zita-resampler/vresampler.h


$(ZITA-RESAMPLER_MIN): $(ZITA-RESAMPLER_O)
	g++ -shared $(LDFLAGS) -Wl,-soname,$(ZITA-RESAMPLER_MAJ) -o $(ZITA-RESAMPLER_MIN) $(ZITA-RESAMPLER_O) $(ZITA-RESAMPLER_DEP)

$(ZITA-RESAMPLER_O):	$(ZITA-RESAMPLER_H)


install:	$(ZITA-RESAMPLER_MIN)
	install -d $(DESTDIR)$(PREFIX)/include/zita-resampler
	install -d $(DESTDIR)$(PREFIX)/$(LIBDIR)
	install -m 644 $(ZITA-RESAMPLER_H) $(DESTDIR)$(PREFIX)/include/zita-resampler
	install -m 644 $(ZITA-RESAMPLER_MIN) $(DESTDIR)$(PREFIX)/$(LIBDIR)
	ln -sf $(ZITA-RESAMPLER_MIN) $(DESTDIR)$(PREFIX)/$(LIBDIR)/$(ZITA-RESAMPLER_SO)
	ldconfig

uninstall:
	/bin/rm -rf $(DESTDIR)$(PREFIX)/include/zita-resampler
	/bin/rm -rf $(DESTDIR)$(PREFIX)/$(LIBDIR)/libzita-resampler*

clean:
	/bin/rm -rf *~ *.o *.a *.d *.so.* zita-resampler/*~


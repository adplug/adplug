# AdPlug Makefile, (c) 2001 Simon Peter <dn.tlp@gmx.net>

CC = cc
CXX = c++
INSTALL = install

CFLAGS = -Wall
CXXFLAGS = -Wall
CPPFLAGS = -Dstricmp=strcasecmp
LDFLAGS = -shared

OBJS = adplug.o emuopl.o fmopl.o
AUX = makefile makefile.wat adplug.dsp COPYING CREDITS INSTALL README PLAYER_SDK CHANGES

prefix = /usr/local
libdir = $(prefix)/lib
includedir = /usr/include/adplug
distname = adplug-1.1

all: libadplug.so

clean:
	make -C players clean
	rm -f $(OBJS) libadplug.so

distclean: clean

dist:
	-rm -rf $(distname)
	mkdir $(distname)
	mkdir $(distname)/players
	cp *.cpp *.c *.h $(AUX) $(distname)
	cp players/*.cpp players/*.h players/makefile players/makefile.wat $(distname)/players
	tar cfj $(distname).tar.bz2 $(distname)
	rm -rf $(distname)

install: all
	$(INSTALL) -d $(libdir) $(includedir)/players
	$(INSTALL) libadplug.so $(libdir)
	$(INSTALL) -m 644 *.h $(includedir)
	$(INSTALL) -m 644 players/*.h $(includedir)/players

uninstall:
	rm -rf $(includedir)
	rm -f $(libdir)/libadplug.so

libadplug.so: $(OBJS)
	make -C players
	$(CXX) $(LDFLAGS) -o libadplug.so $(OBJS) players/*.o

adplug.o: adplug.cpp adplug.h players/*.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -Iplayers -c -o adplug.o adplug.cpp

emuopl.o: emuopl.cpp emuopl.h fmopl.o opl.h

fmopl.o: fmopl.c fmopl.h fm.h

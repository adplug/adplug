# AdPlug Makefile for WATCOM v11, (c) 2001 Simon Peter <dn.tlp@gmx.net>

CC = wcc386
CXX = wpp386

CFLAGS = -oneatx -oh -ei -zp8 -5 -fpi87 -fp5 -zq
CXXFLAGS = -oneatx -oh -oi+ -ei -zp8 -5 -fpi87 -fp5 -zq
CPPFLAGS = -dstd= -dstring=String

OBJS = adplug.obj emuopl.obj fmopl.obj realopl.obj analopl.obj

.c.obj:
	$(CC) $(CFLAGS) $(CPPFLAGS) $[.

.cpp.obj:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $[.

all: adplug.lib

clean: .symbolic
	cd players
	wmake /f makefile.wat clean
	cd ..
	del *.obj
	del adplug.lib

adplug.lib: $(OBJS)
	cd players
	wmake /f makefile.wat
	cd ..
	wlib -n -b adplug.lib +adplug +emuopl +fmopl +analopl +realopl +players\players.lib

adplug.obj: adplug.cpp adplug.h players/*.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -i=players adplug.cpp

emuopl.obj: emuopl.cpp emuopl.h fmopl.obj opl.h

realopl.obj: realopl.cpp realopl.h

analopl.obj: analopl.cpp analopl.h

fmopl.obj: fmopl.c fmopl.h

lib_LTLIBRARIES = libadplug.la

# Build library
libadplug_la_SOURCES = adplug.cpp emuopl.cpp fmopl.c \
players/a2m.cpp players/amd.cpp players/bam.cpp players/d00.cpp players/dfm.cpp \
players/hsc.cpp players/hsp.cpp players/imf.cpp players/ksm.cpp \
players/lds.cpp players/mid.cpp players/mkj.cpp players/mtk.cpp \
players/protrack.cpp players/rad.cpp players/raw.cpp \
players/s3m.cpp players/sa2.cpp players/sng.cpp players/u6m.cpp \
players/fmc.cpp players/mad.cpp players/xad.cpp

libadplug_la_LDFLAGS = -version-info 1:2

AM_CPPFLAGS = -Dstricmp=strcasecmp
CFLAGS = -Wall
CXXFLAGS = -Wall
INCLUDES = -Iplayers -Iplayers/xad

# Install headers
pkginclude_HEADERS = adplug.h emuopl.h fmopl.h silentopl.h opl.h silentopl.h \
players/a2m.h players/amd.h players/bam.h players/d00.h players/dfm.h \
players/hsc.h players/hsp.h players/imf.h players/ksm.h players/lds.h \
players/mid.h players/mkj.h players/mtk.h players/protrack.h players/rad.h \
players/raw.h players/s3m.h players/sa2.h players/sng.h players/u6m.h \
players/imfcrc.h players/mididata.h players/player.h players/fmc.h \
players/mad.h players/xad.h

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

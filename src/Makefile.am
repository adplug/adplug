lib_LTLIBRARIES = libadplug.la

libadplug_la_SOURCES = adplug.cpp emuopl.cpp fmopl.c diskopl.cpp \
a2m.cpp amd.cpp bam.cpp d00.cpp dfm.cpp hsc.cpp hsp.cpp imf.cpp ksm.cpp \
lds.cpp mid.cpp mkj.cpp mtk.cpp protrack.cpp rad.cpp raw.cpp s3m.cpp sa2.cpp \
sng.cpp u6m.cpp fmc.cpp mad.cpp xad.cpp bmf.cpp flash.cpp hyp.cpp psi.cpp \
rat.cpp hybrid.cpp rol.cpp debug.c debug.h mididata.h imfcrc.h adtrack.cpp \
cff.cpp dtm.cpp dmo.cpp

libadplug_la_LDFLAGS = -release @VERSION@

AM_CPPFLAGS = -Dstricmp=strcasecmp
AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall

pkginclude_HEADERS = adplug.h emuopl.h fmopl.h silentopl.h opl.h diskopl.h \
a2m.h amd.h bam.h d00.h dfm.h hsc.h hsp.h imf.h ksm.h lds.h mid.h mkj.h \
mtk.h protrack.h rad.h raw.h s3m.h sa2.h sng.h u6m.h player.h fmc.h mad.h \
xad.h bmf.h flash.h hyp.h psi.h rat.h hybrid.h rol.h adtrack.h cff.h dtm.h \
dmo.h

EXTRA_DIST = Makefile.bt adplug.dsp analopl.cpp analopl.h realopl.cpp \
realopl.h vc6inst.bat

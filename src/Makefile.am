lib_LTLIBRARIES = libadplug.la

libadplug_la_SOURCES = adplug.cpp emuopl.cpp fmopl.c diskopl.cpp debug.c \
debug.h fprovide.cpp player.cpp database.cpp hsc.cpp sng.cpp imf.cpp \
players.cpp protrack.cpp a2m.cpp

libadplug_la_LDFLAGS = -release @VERSION@

# This is a hack. Throughout AdPlug, stricmp() is used to do caseless string
# comparations. UNIX libc's don't support stricmp(), but do support the BSD
# strcasecmp(), so we have to replace every occurance of stricmp() into
# strcasecmp(). stricmp() seems to be Windows, but maybe also POSIX...
AM_CPPFLAGS = -Dstricmp=strcasecmp

pkginclude_HEADERS = adplug.h emuopl.h fmopl.h silentopl.h opl.h diskopl.h \
a2m.h amd.h bam.h d00.h dfm.h hsc.h hsp.h imf.h ksm.h lds.h mid.h mkj.h \
mtk.h protrack.h rad.h raw.h s3m.h sa2.h sng.h u6m.h player.h fmc.h mad.h \
xad.h bmf.h flash.h hyp.h psi.h rat.h hybrid.h rol.h adtrack.h cff.h dtm.h \
dmo.h fprovide.h database.h players.h

EXTRA_DIST = Makefile.bt adplug.dsp analopl.cpp analopl.h realopl.cpp \
realopl.h vc6inst.bat

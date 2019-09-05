EXTRA_DIST = adplug.spec adplug.qpg BUGS adplug.pc.in

pkgconfig_DATA = adplug.pc

########
# src/ #
########

lib_LTLIBRARIES = src/libadplug.la

src_libadplug_la_SOURCES = \
	src/debug.h \
	src/mididata.h \
	src/adlibemu.c \
	src/debug.c \
	src/fmopl.c \
	src/nukedopl.c \
	src/a2m.cpp \
	src/adl.cpp \
	src/adlib.cpp \
	src/adplug.cpp \
	src/adtrack.cpp \
	src/amd.cpp \
	src/analopl.cpp \
	src/bam.cpp \
	src/bmf.cpp \
	src/cff.cpp \
	src/cmf.cpp \
	src/cmfmcsop.cpp \
	src/d00.cpp \
	src/database.cpp \
	src/dfm.cpp \
	src/diskopl.cpp \
	src/dmo.cpp \
	src/dro2.cpp \
	src/dro.cpp \
	src/dtm.cpp \
	src/emuopl.cpp \
	src/flash.cpp \
	src/fmc.cpp \
	src/fprovide.cpp \
	src/got.cpp \
	src/herad.cpp \
	src/hsc.cpp \
	src/hsp.cpp \
	src/hybrid.cpp \
	src/hyp.cpp \
	src/imf.cpp \
	src/jbm.cpp \
	src/ksm.cpp \
	src/lds.cpp \
	src/mad.cpp \
	src/mdi.cpp \
	src/mid.cpp \
	src/mkj.cpp \
	src/msc.cpp \
	src/mtk.cpp \
	src/mus.cpp \
	src/nemuopl.cpp \
	src/player.cpp \
	src/players.cpp \
	src/protrack.cpp \
	src/psi.cpp \
	src/rad.cpp \
	src/rat.cpp \
	src/raw.cpp \
	src/realopl.cpp \
	src/rix.cpp \
	src/rol.cpp \
	src/s3m.cpp \
	src/sa2.cpp \
	src/sng.cpp \
	src/sop.cpp \
	src/surroundopl.cpp \
	src/temuopl.cpp \
	src/u6m.cpp \
	src/vgm.cpp \
	src/woodyopl.cpp \
	src/xad.cpp \
	src/xsm.cpp

src_libadplug_la_LDFLAGS = -release $(PACKAGE_VERSION) -version-info 0
src_libadplug_la_LIBADD = $(libbinio_LIBS)

# -Dstricmp=strcasecmp is a hack. Throughout AdPlug, stricmp() is used to do
# caseless string comparations. UNIX libcs don't support stricmp(), but do
# support the BSD strcasecmp(), so we have to replace every occurence of
# stricmp() into strcasecmp(). stricmp() seems to be Windows, but maybe
# also POSIX...
src_libadplug_la_CPPFLAGS = -Dstricmp=strcasecmp $(libbinio_CFLAGS) -I$(top_builddir)/src

pkginclude_HEADERS = \
	src/adplug.h \
	src/emuopl.h \
	src/fmopl.h \
	src/silentopl.h \
	src/opl.h \
	src/diskopl.h \
	src/a2m.h \
	src/amd.h \
	src/bam.h \
	src/d00.h \
	src/dfm.h \
	src/hsc.h \
	src/hsp.h \
	src/imf.h \
	src/ksm.h \
	src/lds.h \
	src/mid.h \
	src/mkj.h \
	src/mtk.h \
	src/protrack.h \
	src/rad.h \
	src/raw.h \
	src/s3m.h \
	src/sa2.h \
	src/sng.h \
	src/u6m.h \
	src/player.h \
	src/fmc.h \
	src/mad.h \
	src/xad.h \
	src/bmf.h \
	src/flash.h \
	src/hyp.h \
	src/psi.h \
	src/rat.h \
	src/hybrid.h \
	src/rol.h \
	src/adtrack.h \
	src/cff.h \
	src/dtm.h \
	src/dmo.h \
	src/fprovide.h \
	src/database.h \
	src/players.h \
	src/xsm.h \
	src/adlibemu.h \
	src/kemuopl.h \
	src/dro.h \
	src/realopl.h \
	src/analopl.h \
	src/temuopl.h \
	src/msc.h \
	src/rix.h \
	src/adl.h \
	src/jbm.h \
	src/cmf.h \
	src/surroundopl.h \
	src/dro2.h \
	src/got.h \
	src/version.h \
	src/wemuopl.h \
	src/woodyopl.h \
	src/nemuopl.h \
	src/nukedopl.h \
	src/adlib.h \
	src/mus.h \
	src/mdi.h \
	src/cmfmcsop.h \
	src/vgm.h \
	src/sop.h \
	src/herad.h \
	src/load_helper.h

#############
# adplugdb/ #
#############

bin_PROGRAMS = adplugdb/adplugdb

adplugdb_adplugdb_SOURCES = adplugdb/adplugdb.cpp
EXTRA_adplugdb_adplugdb_SOURCES = adplugdb/getopt.c adplugdb/mygetopt.h

adplugdb_adplugdb_CPPFLAGS = -DADPLUG_DATA_DIR=\"$(adplug_data_dir)\" -I$(top_srcdir)/src $(libbinio_CFLAGS)
adplugdb_adplugdb_LDADD = $(GETOPT_OBJECTS) src/libadplug.la $(libbinio_LIBS)
adplugdb_adplugdb_DEPENDENCIES = $(GETOPT_OBJECTS) src/libadplug.la

adplugdb_adplugdb_data_dir = $(sharedstatedir)/adplug

########
# doc/ #
########

info_TEXINFOS = doc/libadplug.texi

doc_libadplug_TEXINFOS = doc/fdl.texi

dist_man_MANS = doc/adplugdb.1

EXTRA_DIST += doc/adplugdb.1.in

MOSTLYCLEANFILES = \
	doc/stamp-vti \
	doc/libadplug.info \
	doc/libadplug.info-1 \
	doc/libadplug.info-2

CLEANFILES = \
	doc/libadplug.cps \
	doc/libadplug.fns \
	doc/libadplug.vrs

DISTCLEANFILES = doc/adplugdb.1

MAINTAINERCLEANFILES = doc/version.texi

ADPLUGDB_1_EDIT = $(SED) \
	-e 's,[@]sharedstatedir[@],$(sharedstatedir),g' \
	-e 's,[@]VERSION[@],$(VERSION),g'

# It is tempting to use $< in this rule. Don't do that!
# Using $< in ordinary Make rules is not portable,
# in the case of FreeBSD's pmake the variable is empty
# and OpenBSD's make errors out.
# https://www.gnu.org/software/autoconf/manual/autoconf.html#g_t_0024_003c-in-Ordinary-Make-Rules
doc/adplugdb.1: $(top_srcdir)/doc/adplugdb.1.in
	cd $(top_srcdir) && \
	$(MKDIR_P) doc && \
	cd doc && \
	rm -f adplugdb.1 adplugdb.1.tmp && \
	$(ADPLUGDB_1_EDIT) adplugdb.1.in > adplugdb.1.tmp && \
	mv adplugdb.1.tmp adplugdb.1

#########
# test/ #
#########

check_PROGRAMS = \
	test/playertest \
	test/emutest \
	test/crctest

test_playertest_SOURCES = test/playertest.cpp
test_emutest_SOURCES = test/emutest.cpp
test_crctest_SOURCES = test/crctest.cpp

test_playertest_CPPFLAGS = $(libbinio_CFLAGS)
test_emutest_CPPFLAGS = $(libbinio_CFLAGS)
test_crctest_CPPFLAGS = $(libbinio_CFLAGS)

test_playertest_LDADD = src/libadplug.la $(libbinio_LIBS)
test_emutest_LDADD = src/libadplug.la $(libbinio_LIBS)
test_crctest_LDADD = src/libadplug.la $(libbinio_LIBS)

AM_TESTS_ENVIRONMENT = testdir='$(srcdir)/test'; export testdir;
TESTS = $(check_PROGRAMS)

EXTRA_DIST += \
	test/2001.MKJ \
	test/2001.ref \
	test/2.CMF \
	test/2.ref \
	test/ADAGIO.DFM \
	test/ADAGIO.ref \
	test/adlibsp.ref \
	test/adlibsp.s3m \
	test/ALLOYRUN.RAD \
	test/ALLOYRUN.ref \
	test/ARAB.BAM \
	test/ARAB.ref \
	test/ARRAKIS.ref \
	test/ARRAKIS.SDB \
	test/BEGIN.KSM \
	test/BEGIN.ref \
	test/BeyondSN.ref \
	test/BeyondSN.vgm \
	test/blaster2.msc \
	test/blaster2.ref \
	test/bmf1_1.bmf \
	test/bmf1_1.ref \
	test/bmf1_2.ref \
	test/bmf1_2.xad \
	test/BOOTUP.M \
	test/BOOTUP.ref \
	test/CHILD1.ref \
	test/CHILD1.XSM \
	test/crusader.raw \
	test/crusader.ref \
	test/DEMO4.JBM \
	test/DEMO4.ref \
	test/doofus.dro \
	test/doofus.ref \
	test/dro_v2.dro \
	test/dro_v2.ref \
	test/DTM-TRK1.DTM \
	test/DTM-TRK1.ref \
	test/DUNE19.ADL \
	test/DUNE19.ref \
	test/ending.ref \
	test/ending.sop \
	test/EOBSOUND.ADL \
	test/EOBSOUND.ref \
	test/fdance03.dmo \
	test/fdance03.ref \
	test/flash.ref \
	test/flash.xad \
	test/Flying.mdi \
	test/Flying.ref \
	test/GALWAY.ref \
	test/GALWAY.SOP \
	test/go-_-go.bnk \
	test/go-_-go.ims \
	test/go-_-go.ref \
	test/GORBI2.ref \
	test/GORBI2.SQX \
	test/HIP_D.ref \
	test/HIP_D.ROL \
	test/hybrid.ref \
	test/hybrid.xad \
	test/hyp.ref \
	test/hyp.xad \
	test/icepatch.003 \
	test/ice_thnk.ref \
	test/ice_thnk.sci \
	test/implay.bnk \
	test/inc.raw \
	test/inc.ref \
	test/insts.dat \
	test/lines1.mus \
	test/lines1.ref \
	test/lines1.snd \
	test/LOREINTR.ADL \
	test/LOREINTR.ref \
	test/loudness.lds \
	test/loudness.ref \
	test/MainBGM5.ref \
	test/MainBGM5.vgm \
	test/MARIO.A2M \
	test/MARIO.ref \
	test/menu.got \
	test/menu.ref \
	test/mi2.laa \
	test/mi2.ref \
	test/michaeld.cmf \
	test/michaeld.ref \
	test/MORNING.HSQ \
	test/MORNING.ref \
	test/NECRONOM.CMF \
	test/NECRONOM.ref \
	test/NEWPAGA.HA2 \
	test/NEWPAGA.ref \
	test/NEWSAN.HSQ \
	test/NEWSAN.ref \
	test/opensong.got \
	test/opensong.ref \
	test/PLAYMUS1.ref \
	test/PLAYMUS1.SNG \
	test/psi1.ref \
	test/psi1.xad \
	test/rat.ref \
	test/rat.xad \
	test/REVELAT.ref \
	test/REVELAT.SNG \
	test/revival.ims \
	test/revival.ref \
	test/RI051.ref \
	test/RI051.RIX \
	test/RIK6.MDI \
	test/RIK6.ref \
	test/SAILOR.CFF \
	test/SAILOR.ref \
	test/samurai.dro \
	test/samurai.ref \
	test/SATNIGHT.HSP \
	test/SATNIGHT.ref \
	test/SCALES.ref \
	test/SCALES.SA2 \
	test/SMKEREM.HSC \
	test/SMKEREM.ref \
	test/SONG1.ins \
	test/SONG1.ref \
	test/SONG1.sng \
	test/standard.bnk \
	test/tafa.mus \
	test/tafa.ref \
	test/tafa.tim \
	test/TOCCATA.MAD \
	test/TOCCATA.ref \
	test/TUBES.ref \
	test/TUBES.SAT \
	test/TU_BLESS.AMD \
	test/TU_BLESS.ref \
	test/VIB_VOL3.D00 \
	test/VIB_VOL3.ref \
	test/WONDERIN.ref \
	test/WONDERIN.WLF \
	test/WORMINTR.AGD \
	test/WORMINTR.ref \
	test/YsBattle.ref \
	test/YsBattle.vgm

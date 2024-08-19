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
	src/depack.c \
	src/sixdepack.cpp \
	src/unlzh.c \
	src/unlzss.c \
	src/unlzw.c \
	src/a2m.cpp \
	src/a2m-v2.cpp \
	src/adl.cpp \
	src/adplug.cpp \
	src/adtrack.cpp \
	src/amd.cpp \
	src/analopl.cpp \
	src/bam.cpp \
	src/bmf.cpp \
	src/cff.cpp \
	src/cmf.cpp \
	src/cmfmcsop.cpp \
	src/coktel.cpp \
	src/composer.cpp \
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
	src/kemuopl.cpp \
	src/ksm.cpp \
	src/lds.cpp \
	src/mad.cpp \
	src/mdi.cpp \
	src/mid.cpp \
	src/mkj.cpp \
	src/msc.cpp \
	src/mtk.cpp \
	src/mtr.cpp \
	src/mus.cpp \
	src/nemuopl.cpp \
	src/pis.cpp \
	src/player.cpp \
	src/players.cpp \
	src/protrack.cpp \
	src/psi.cpp \
	src/rad2.cpp \
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
src_libadplug_la_LDFLAGS += -no-undefined # mingw requires this when making shared DLL files
src_libadplug_la_LIBADD = $(libbinio_LIBS)

# -Dstricmp=strcasecmp is a hack. Throughout AdPlug, stricmp() is used to do
# caseless string comparations. UNIX libcs don't support stricmp(), but do
# support the BSD strcasecmp(), so we have to replace every occurrence of
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
	src/depack.h \
	src/sixdepack.h \
	src/unlzh.h \
	src/unlzss.h \
	src/unlzw.h \
	src/a2m.h \
	src/a2m-v2.h \
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
	src/mtr.h \
	src/protrack.h \
	src/rad2.h \
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
	src/pis.h \
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
	src/mus.h \
	src/mdi.h \
	src/cmfmcsop.h \
	src/coktel.h \
	src/composer.h \
	src/vgm.h \
	src/sop.h \
	src/herad.h \
	src/strnlen.h \
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
	test/playtest \
	test/emutest \
	test/crctest \
	test/strstest

test_playtest_SOURCES = test/playertest.cpp
test_emutest_SOURCES = test/emutest.cpp
test_crctest_SOURCES = test/crctest.cpp
test_strstest_SOURCES = test/stresstest.cpp

test_playtest_CPPFLAGS = $(libbinio_CFLAGS)
test_emutest_CPPFLAGS = $(libbinio_CFLAGS)
test_crctest_CPPFLAGS = $(libbinio_CFLAGS)
test_strstest_CPPFLAGS = $(libbinio_CFLAGS)

test_playtest_LDADD = src/libadplug.la $(libbinio_LIBS)
test_emutest_LDADD = src/libadplug.la $(libbinio_LIBS)
test_crctest_LDADD = src/libadplug.la $(libbinio_LIBS)
test_strstest_LDADD = src/libadplug.la $(libbinio_LIBS)

AM_TESTS_ENVIRONMENT = testdir='$(srcdir)/test'; export testdir;
TESTS = $(check_PROGRAMS)

EXTRA_DIST += \
	test/testmus/2001.MKJ \
	test/testref/2001.ref \
	test/testmus/2.CMF \
	test/testref/2.ref \
	test/testmus/AB_JULIA.A2T \
	test/testref/AB_JULIA.ref \
	test/testmus/ACTION.PIS \
	test/testref/ACTION.ref \
	test/testmus/ADAGIO.DFM \
	test/testref/ADAGIO.ref \
	test/testref/adlibsp.ref \
	test/testmus/adlibsp.s3m \
	test/testmus/AKMTEC.MTR \
	test/testref/AKMTEC.ref \
	test/testmus/ALLOYRUN.RAD \
	test/testref/ALLOYRUN.ref \
	test/testmus/ARAB.BAM \
	test/testref/ARAB.ref \
	test/testref/ARRAKIS.ref \
	test/testmus/ARRAKIS.SDB \
	test/testmus/BEGIN.KSM \
	test/testref/BEGIN.ref \
	test/testref/BeyondSN.ref \
	test/testmus/BeyondSN.vgm \
	test/testmus/blaster2.msc \
	test/testref/blaster2.ref \
	test/testmus/bmf1_1.bmf \
	test/testref/bmf1_1.ref \
	test/testref/bmf1_2.ref \
	test/testmus/bmf1_2.xad \
	test/testmus/BOOTUP.M \
	test/testref/BOOTUP.ref \
	test/testmus/canonind.rad \
	test/testref/canonind.ref \
	test/testref/CHILD1.ref \
	test/testmus/CHILD1.XSM \
	test/testmus/CHIP.MTR \
	test/testref/CHIP.ref \
	test/testmus/crusader.raw \
	test/testref/crusader.ref \
	test/testmus/DEMO4.JBM \
	test/testref/DEMO4.ref \
	test/testmus/doofus.dro \
	test/testref/doofus.ref \
	test/testmus/dro_v2.dro \
	test/testref/dro_v2.ref \
	test/testmus/DTM-TRK1.DTM \
	test/testref/DTM-TRK1.ref \
	test/testmus/DUNE19.ADL \
	test/testref/DUNE19.ref \
	test/testmus/dystopia.rad \
	test/testref/dystopia.ref \
	test/testref/ending.ref \
	test/testmus/ending.sop \
	test/testmus/EOBSOUND.ADL \
	test/testref/EOBSOUND.ref \
	test/testmus/fank5.a2m \
	test/testref/fank5.ref \
	test/testmus/fdance03.dmo \
	test/testref/fdance03.ref \
	test/testref/flash.ref \
	test/testmus/flash.xad \
	test/testmus/Flying.mdi \
	test/testref/Flying.ref \
	test/testmus/fm-troni.a2m \
	test/testref/fm-troni.ref \
	test/testref/GALWAY.ref \
	test/testmus/GALWAY.SOP \
	test/testmus/go-_-go.bnk \
	test/testmus/go-_-go.ims \
	test/testref/go-_-go.ref \
	test/testref/GORBI2.ref \
	test/testmus/GORBI2.SQX \
	test/testmus/GRABBAG.MID \
	test/testref/GRABBAG.ref \
	test/testref/HIP_D.ref \
	test/testmus/HIP_D.ROL \
	test/testref/hybrid.ref \
	test/testmus/hybrid.xad \
	test/testref/hyp.ref \
	test/testmus/hyp.xad \
	test/testmus/icepatch.003 \
	test/testref/ice_thnk.ref \
	test/testmus/ice_thnk.sci \
	test/testmus/implay.bnk \
	test/testmus/inc.raw \
	test/testref/inc.ref \
	test/testmus/insts.dat \
	test/testmus/lines1.mus \
	test/testref/lines1.ref \
	test/testmus/lines1.snd \
	test/testmus/LOREINTR.ADL \
	test/testref/LOREINTR.ref \
	test/testmus/loudness.lds \
	test/testref/loudness.ref \
	test/testref/MainBGM5.ref \
	test/testmus/MainBGM5.vgm \
	test/testmus/MARIO.A2M \
	test/testref/MARIO.ref \
	test/testmus/menu.got \
	test/testref/menu.ref \
	test/testmus/mi2.laa \
	test/testref/mi2.ref \
	test/testmus/michaeld.cmf \
	test/testref/michaeld.ref \
	test/testmus/MORNING.HSQ \
	test/testref/MORNING.ref \
	test/testmus/NECRONOM.CMF \
	test/testref/NECRONOM.ref \
	test/testmus/SNDTRACK.CMF \
	test/testref/SNDTRACK.ref \
	test/testmus/NEWPAGA.HA2 \
	test/testref/NEWPAGA.ref \
	test/testmus/NEWSAN.HSQ \
	test/testref/NEWSAN.ref \
	test/testmus/opensong.got \
	test/testref/opensong.ref \
	test/testref/PLAYMUS1.ref \
	test/testmus/PLAYMUS1.SNG \
	test/testref/psi1.ref \
	test/testmus/psi1.xad \
	test/testref/rat.ref \
	test/testmus/rat.xad \
	test/testref/REVELAT.ref \
	test/testmus/REVELAT.SNG \
	test/testmus/revival.ims \
	test/testref/revival.ref \
	test/testref/RI051.ref \
	test/testmus/RI051.RIX \
	test/testmus/RIK6.MDI \
	test/testref/RIK6.ref \
	test/testmus/SAILOR.CFF \
	test/testref/SAILOR.ref \
	test/testmus/samurai.dro \
	test/testref/samurai.ref \
	test/testmus/SATNIGHT.HSP \
	test/testref/SATNIGHT.ref \
	test/testmus/SAVAGE.HSQ \
	test/testref/SAVAGE.ref \
	test/testref/SCALES.ref \
	test/testmus/SCALES.SA2 \
	test/testmus/SMKEREM.HSC \
	test/testref/SMKEREM.ref \
	test/testmus/SONG1.ins \
	test/testref/SONG1.ref \
	test/testmus/SONG1.sng \
	test/testmus/standard.bnk \
	test/testmus/tafa.mus \
	test/testref/tafa.ref \
	test/testmus/tafa.tim \
	test/testmus/TEST16.MID \
	test/testref/TEST16.ref \
	test/testmus/TheAlibi.d00 \
	test/testref/TheAlibi.ref \
	test/testmus/TOCCATA.MAD \
	test/testref/TOCCATA.ref \
	test/testref/TUBES.ref \
	test/testmus/TUBES.SAT \
	test/testmus/TU_BLESS.AMD \
	test/testref/TU_BLESS.ref \
	test/testmus/VIB_VOL3.D00 \
	test/testref/VIB_VOL3.ref \
	test/testmus/well.adl \
	test/testref/well.ref \
	test/testref/WONDERIN.ref \
	test/testmus/WONDERIN.WLF \
	test/testmus/WORMINTR.AGD \
	test/testref/WORMINTR.ref \
	test/testref/YsBattle.ref \
	test/testmus/YsBattle.vgm
# Regression tests:
EXTRA_DIST += \
	test/fuzzing/i-85.bmf \
	test/fuzzing/i-86.dtm \
	test/fuzzing/i-87.mkj \
	test/fuzzing/i-88_1.a2m test/fuzzing/i-88_2.a2m \
	test/fuzzing/i-89_1.rad test/fuzzing/i-89_2.rad \
	test/fuzzing/i-90_1.mtk test/fuzzing/i-90_2.mtk \
	test/fuzzing/i-91.m \
	test/fuzzing/i-92_1.m test/fuzzing/i-92_2.m test/fuzzing/i-92_3.m test/fuzzing/i-92_4.m \
	test/fuzzing/i-93_01.bmf test/fuzzing/i-93_02.bmf test/fuzzing/i-93_03.xad test/fuzzing/i-93_04.bmf \
	test/fuzzing/i-93_05.xad test/fuzzing/i-93_06.xad test/fuzzing/i-93_07.bmf test/fuzzing/i-93_08.xad \
	test/fuzzing/i-93_09.xad test/fuzzing/i-93_10.xad test/fuzzing/i-93_11.xad test/fuzzing/i-93_12.xad \
	test/fuzzing/i-93_13.xad test/fuzzing/i-93_14.xad test/fuzzing/i-93_15.bmf test/fuzzing/i-93_16.xad \
	test/fuzzing/i-93_17.xad test/fuzzing/i-93_18.xad \
	test/fuzzing/i-94.rix \
	test/fuzzing/i-95_1.ctm test/fuzzing/i-95_2.ctm \
	test/fuzzing/i-96_1.rol	test/fuzzing/i-96_2.rol	test/fuzzing/i-96_3.rol \
	test/fuzzing/i-98.sng \
	test/fuzzing/i-99.dro \
	test/fuzzing/i-100_01.xad test/fuzzing/i-100_02.xad test/fuzzing/i-100_03.dro \
	test/fuzzing/i-100_04.dro test/fuzzing/i-100_05.raw test/fuzzing/i-100_06.raw \
	test/fuzzing/i-100_07.dro test/fuzzing/i-100_08.dro test/fuzzing/i-100_09.adlib \
	test/fuzzing/i-100_10.mkj test/fuzzing/i-100_11.mkj test/fuzzing/i-100_12.cmf \
	test/fuzzing/i-100_13.cmf \
	test/fuzzing/i-101_1.d00 test/fuzzing/i-101_2.d00 \
	test/fuzzing/i-102_001.dfm test/fuzzing/i-102_002.dfm test/fuzzing/i-102_003.dfm \
	test/fuzzing/i-102_004.dfm test/fuzzing/i-102_005.dfm test/fuzzing/i-102_006.dfm \
	test/fuzzing/i-102_007.dfm test/fuzzing/i-102_008.dfm test/fuzzing/i-102_009.dfm \
	test/fuzzing/i-102_010.dfm test/fuzzing/i-102_011.dfm test/fuzzing/i-102_012.dfm \
	test/fuzzing/i-102_013.dfm test/fuzzing/i-102_014.dfm test/fuzzing/i-102_015.dfm \
	test/fuzzing/i-102_016.dfm test/fuzzing/i-102_017.dfm test/fuzzing/i-102_018.dfm \
	test/fuzzing/i-102_019.dfm test/fuzzing/i-102_020.dfm test/fuzzing/i-102_021.dfm \
	test/fuzzing/i-102_022.dfm test/fuzzing/i-102_023.dfm test/fuzzing/i-102_024.dfm \
	test/fuzzing/i-102_025.dfm test/fuzzing/i-102_026.dfm test/fuzzing/i-102_027.dfm \
	test/fuzzing/i-102_028.dfm test/fuzzing/i-102_029.dfm test/fuzzing/i-102_030.dfm \
	test/fuzzing/i-102_031.dfm test/fuzzing/i-102_032.dfm test/fuzzing/i-102_033.dfm \
	test/fuzzing/i-102_034.dfm test/fuzzing/i-102_035.dfm test/fuzzing/i-102_036.dfm \
	test/fuzzing/i-102_037.dfm test/fuzzing/i-102_038.dfm test/fuzzing/i-102_039.dfm \
	test/fuzzing/i-102_040.dfm test/fuzzing/i-102_041.dfm test/fuzzing/i-102_042.dfm \
	test/fuzzing/i-102_043.dfm test/fuzzing/i-102_044.dfm test/fuzzing/i-102_045.dfm \
	test/fuzzing/i-102_046.dfm test/fuzzing/i-102_047.dfm test/fuzzing/i-102_048.dfm \
	test/fuzzing/i-102_049.dfm test/fuzzing/i-102_050.dfm test/fuzzing/i-102_051.dfm \
	test/fuzzing/i-102_052.dfm test/fuzzing/i-102_053.dfm test/fuzzing/i-102_054.dfm \
	test/fuzzing/i-102_055.dfm test/fuzzing/i-102_056.dfm test/fuzzing/i-102_057.dfm \
	test/fuzzing/i-102_058.dfm test/fuzzing/i-102_059.dfm test/fuzzing/i-102_060.dfm \
	test/fuzzing/i-102_061.dfm test/fuzzing/i-102_062.dfm test/fuzzing/i-102_063.dfm \
	test/fuzzing/i-102_064.dfm test/fuzzing/i-102_065.dfm test/fuzzing/i-102_066.dfm \
	test/fuzzing/i-102_067.dfm test/fuzzing/i-102_068.dfm test/fuzzing/i-102_069.dfm \
	test/fuzzing/i-102_070.dfm test/fuzzing/i-102_071.dfm test/fuzzing/i-102_072.dfm \
	test/fuzzing/i-102_073.dfm test/fuzzing/i-102_074.dfm test/fuzzing/i-102_075.dfm \
	test/fuzzing/i-102_076.dfm test/fuzzing/i-102_077.dfm test/fuzzing/i-102_078.dfm \
	test/fuzzing/i-102_079.dfm test/fuzzing/i-102_080.dfm test/fuzzing/i-102_081.dfm \
	test/fuzzing/i-102_082.dfm test/fuzzing/i-102_083.dfm test/fuzzing/i-102_084.dfm \
	test/fuzzing/i-102_085.dfm test/fuzzing/i-102_086.dfm test/fuzzing/i-102_087.dfm \
	test/fuzzing/i-102_088.dfm test/fuzzing/i-102_089.dfm test/fuzzing/i-102_090.dfm \
	test/fuzzing/i-102_091.dfm test/fuzzing/i-102_092.dfm test/fuzzing/i-102_093.dfm \
	test/fuzzing/i-102_094.dfm test/fuzzing/i-102_095.dfm test/fuzzing/i-102_096.dfm \
	test/fuzzing/i-102_097.dfm test/fuzzing/i-102_098.dfm test/fuzzing/i-102_099.dfm \
	test/fuzzing/i-102_100.dfm test/fuzzing/i-102_101.dfm test/fuzzing/i-102_102.dfm \
	test/fuzzing/i-102_103.dfm test/fuzzing/i-102_104.dfm test/fuzzing/i-102_105.dfm \
	test/fuzzing/i-102_106.dfm test/fuzzing/i-102_107.dfm test/fuzzing/i-102_108.dfm \
	test/fuzzing/i-102_109.dfm test/fuzzing/i-102_110.dfm test/fuzzing/i-102_111.dfm \
	test/fuzzing/i-102_112.dfm test/fuzzing/i-102_113.dfm test/fuzzing/i-102_114.dfm \
	test/fuzzing/i-102_115.dfm test/fuzzing/i-102_116.dfm test/fuzzing/i-102_117.dfm \
	test/fuzzing/i-102_118.dfm test/fuzzing/i-102_119.dfm test/fuzzing/i-102_120.dfm \
	test/fuzzing/i-102_121.dfm test/fuzzing/i-102_122.dfm test/fuzzing/i-102_123.dfm \
	test/fuzzing/i-102_124.dfm test/fuzzing/i-102_125.dfm test/fuzzing/i-102_126.dfm \
	test/fuzzing/i-102_127.dfm test/fuzzing/i-102_128.dfm test/fuzzing/i-102_129.dfm \
	test/fuzzing/i-102_130.dfm test/fuzzing/i-102_131.dfm test/fuzzing/i-102_132.dfm \
	test/fuzzing/i-102_133.dfm test/fuzzing/i-102_134.dfm test/fuzzing/i-102_135.dfm \
	test/fuzzing/i-102_136.dfm test/fuzzing/i-102_137.dfm test/fuzzing/i-102_138.dfm \
	test/fuzzing/i-102_139.dfm test/fuzzing/i-102_140.dfm test/fuzzing/i-102_141.dfm \
	test/fuzzing/i-102_142.dfm test/fuzzing/i-102_143.dfm test/fuzzing/i-102_144.dfm \
	test/fuzzing/i-102_145.dfm test/fuzzing/i-102_146.dfm test/fuzzing/i-102_147.dfm \
	test/fuzzing/i-102_148.dfm test/fuzzing/i-102_149.dfm test/fuzzing/i-102_150.dfm \
	test/fuzzing/i-102_151.dfm \
	test/fuzzing/i-110_001.dtm test/fuzzing/i-110_002.dtm test/fuzzing/i-110_003.dtm \
	test/fuzzing/i-110_004.dtm test/fuzzing/i-110_005.a2m test/fuzzing/i-110_006.a2m \
	test/fuzzing/i-110_007.a2m test/fuzzing/i-110_008.a2m test/fuzzing/i-110_009.a2m \
	test/fuzzing/i-110_010.a2m test/fuzzing/i-110_011.a2m test/fuzzing/i-110_012.cff \
	test/fuzzing/i-110_013.cff test/fuzzing/i-110_014.cff test/fuzzing/i-110_015.cff \
	test/fuzzing/i-110_016.cff test/fuzzing/i-110_017.cff test/fuzzing/i-110_018.cff \
	test/fuzzing/i-110_019.cff test/fuzzing/i-110_020.cff test/fuzzing/i-110_021.cff \
	test/fuzzing/i-110_022.cff test/fuzzing/i-110_023.cff test/fuzzing/i-110_024.cff \
	test/fuzzing/i-110_025.amd test/fuzzing/i-110_026.amd test/fuzzing/i-110_027.amd \
	test/fuzzing/i-110_028.amd test/fuzzing/i-110_029.amd test/fuzzing/i-110_030.xad \
	test/fuzzing/i-110_031.dtm test/fuzzing/i-110_032.dtm test/fuzzing/i-110_033.amd \
	test/fuzzing/i-110_034.xad test/fuzzing/i-110_035.xad test/fuzzing/i-110_036.xad \
	test/fuzzing/i-110_037.xad test/fuzzing/i-110_038.a2m test/fuzzing/i-110_039.dtm \
	test/fuzzing/i-110_040.dtm test/fuzzing/i-110_041.a2m test/fuzzing/i-110_042.a2m \
	test/fuzzing/i-110_043.amd test/fuzzing/i-110_044.a2m test/fuzzing/i-110_045.a2m \
	test/fuzzing/i-110_046.amd test/fuzzing/i-110_047.sng test/fuzzing/i-110_048.cff \
	test/fuzzing/i-110_049.sa2 test/fuzzing/i-110_050.sa2 test/fuzzing/i-110_051.amd \
	test/fuzzing/i-110_052.dtm test/fuzzing/i-110_053.dtm test/fuzzing/i-110_054.dtm \
	test/fuzzing/i-110_055.dtm test/fuzzing/i-110_056.amd test/fuzzing/i-110_057.mtk \
	test/fuzzing/i-110_058.xad test/fuzzing/i-110_059.dtm test/fuzzing/i-110_060.dtm \
	test/fuzzing/i-110_061.dtm test/fuzzing/i-110_062.amd test/fuzzing/i-110_063.dtm \
	test/fuzzing/i-110_064.dtm test/fuzzing/i-110_065.amd test/fuzzing/i-110_066.sng \
	test/fuzzing/i-110_067.sat test/fuzzing/i-110_068.amd test/fuzzing/i-110_069.amd \
	test/fuzzing/i-110_070.xad test/fuzzing/i-110_071.amd test/fuzzing/i-110_072.amd \
	test/fuzzing/i-110_073.a2m test/fuzzing/i-110_074.a2m test/fuzzing/i-110_075.a2m \
	test/fuzzing/i-110_076.a2m test/fuzzing/i-110_077.a2m test/fuzzing/i-110_078.a2m \
	test/fuzzing/i-110_079.sng test/fuzzing/i-110_080.sng test/fuzzing/i-110_081.sa2 \
	test/fuzzing/i-110_082.sa2 test/fuzzing/i-110_083.amd test/fuzzing/i-110_084.amd \
	test/fuzzing/i-110_085.amd test/fuzzing/i-110_086.amd test/fuzzing/i-110_087.amd \
	test/fuzzing/i-110_088.amd test/fuzzing/i-110_089.xad test/fuzzing/i-110_090.xad \
	test/fuzzing/i-110_091.a2m test/fuzzing/i-110_092.cff test/fuzzing/i-110_093.cff \
	test/fuzzing/i-110_094.mtk test/fuzzing/i-110_095.xad test/fuzzing/i-110_096.xad \
	test/fuzzing/i-110_097.cff test/fuzzing/i-110_098.amd test/fuzzing/i-110_099.amd \
	test/fuzzing/i-110_100.dtm test/fuzzing/i-110_101.cff test/fuzzing/i-110_102.cff \
	test/fuzzing/i-110_103.cff test/fuzzing/i-110_104.amd test/fuzzing/i-110_105.cff \
	test/fuzzing/i-110_106.sat test/fuzzing/i-110_107.a2m test/fuzzing/i-110_108.a2m \
	test/fuzzing/i-110_109.amd test/fuzzing/i-110_110.dtm test/fuzzing/i-110_111.dtm \
	test/fuzzing/i-110_112.dtm test/fuzzing/i-110_113.dtm test/fuzzing/i-110_114.dtm \
	test/fuzzing/i-110_115.dtm test/fuzzing/i-110_116.dtm test/fuzzing/i-110_117.dtm \
	test/fuzzing/i-110_118.dtm test/fuzzing/i-110_119.dtm test/fuzzing/i-110_120.dtm \
	test/fuzzing/i-110_121.dtm test/fuzzing/i-110_122.dtm test/fuzzing/i-110_123.xad \
	test/fuzzing/i-110_124.a2m test/fuzzing/i-110_125.dtm test/fuzzing/i-110_126.a2m \
	test/fuzzing/i-110_127.amd test/fuzzing/i-110_128.dtm test/fuzzing/i-110_129.a2m \
	test/fuzzing/i-110_130.amd test/fuzzing/i-110_131.amd test/fuzzing/i-110_132.a2m \
	test/fuzzing/i-110_133.dtm test/fuzzing/i-110_134.sat test/fuzzing/i-110_135.amd \
	test/fuzzing/i-110_136.amd test/fuzzing/i-110_137.a2m test/fuzzing/i-110_138.a2m \
	test/fuzzing/i-110_139.dtm test/fuzzing/i-110_140.sat test/fuzzing/i-110_141.amd \
	test/fuzzing/i-110_142.sat test/fuzzing/i-110_143.dtm test/fuzzing/i-110_144.xad \
	test/fuzzing/i-110_145.amd test/fuzzing/i-110_146.xad test/fuzzing/i-110_147.amd \
	test/fuzzing/i-110_148.sa2 test/fuzzing/i-110_149.sa2 test/fuzzing/i-110_150.sa2 \
	test/fuzzing/i-110_151.amd test/fuzzing/i-110_152.amd test/fuzzing/i-110_153.amd \
	test/fuzzing/i-110_154.amd test/fuzzing/i-110_155.amd test/fuzzing/i-110_156.amd \
	test/fuzzing/i-110_157.a2m test/fuzzing/i-110_158.amd test/fuzzing/i-110_159.a2m \
	test/fuzzing/i-110_160.amd test/fuzzing/i-110_161.xad test/fuzzing/i-110_162.sa2 \
	test/fuzzing/i-110_163.sa2 test/fuzzing/i-110_164.dtm test/fuzzing/i-110_165.dtm \
	test/fuzzing/i-110_166.dtm test/fuzzing/i-110_167.sa2 test/fuzzing/i-110_168.a2m \
	test/fuzzing/i-110_169.amd test/fuzzing/i-110_170.a2m test/fuzzing/i-110_171.amd \
	test/fuzzing/i-110_172.amd test/fuzzing/i-110_173.xad test/fuzzing/i-110_174.amd \
	test/fuzzing/i-110_175.amd test/fuzzing/i-110_176.dtm test/fuzzing/i-110_177.amd \
	test/fuzzing/i-110_178.dtm test/fuzzing/i-110_179.dtm test/fuzzing/i-110_180.a2m \
	test/fuzzing/i-110_181.dtm test/fuzzing/i-110_182.dtm test/fuzzing/i-110_183.a2m \
	test/fuzzing/i-110_184.dtm test/fuzzing/i-110_185.dtm test/fuzzing/i-110_186.a2m \
	test/fuzzing/i-110_187.amd test/fuzzing/i-110_188.mtk test/fuzzing/i-110_189.amd \
	test/fuzzing/i-110_190.amd test/fuzzing/i-110_191.dtm test/fuzzing/i-110_192.a2m \
	test/fuzzing/i-110_193.amd test/fuzzing/i-110_194.dtm test/fuzzing/i-110_195.dtm \
	test/fuzzing/i-110_196.a2m test/fuzzing/i-110_197.sa2 test/fuzzing/i-110_198.dtm \
	test/fuzzing/i-110_199.amd test/fuzzing/i-110_200.a2m

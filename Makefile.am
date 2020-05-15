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
	test/crctest \
	test/stresstest

test_playertest_SOURCES = test/playertest.cpp
test_emutest_SOURCES = test/emutest.cpp
test_crctest_SOURCES = test/crctest.cpp
test_stresstest_SOURCES = test/stresstest.cpp

test_playertest_CPPFLAGS = $(libbinio_CFLAGS)
test_emutest_CPPFLAGS = $(libbinio_CFLAGS)
test_crctest_CPPFLAGS = $(libbinio_CFLAGS)
test_stresstest_CPPFLAGS = $(libbinio_CFLAGS)

test_playertest_LDADD = src/libadplug.la $(libbinio_LIBS)
test_emutest_LDADD = src/libadplug.la $(libbinio_LIBS)
test_crctest_LDADD = src/libadplug.la $(libbinio_LIBS)
test_stresstest_LDADD = src/libadplug.la $(libbinio_LIBS)

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
# Regression tests:
EXTRA_DIST += \
	test/i-85.bmf \
	test/i-86.dtm \
	test/i-87.mkj \
	test/i-88_1.a2m test/i-88_2.a2m \
	test/i-89_1.rad test/i-89_2.rad \
	test/i-90_1.mtk test/i-90_2.mtk \
	test/i-91.m \
	test/i-92_1.m test/i-92_2.m test/i-92_3.m test/i-92_4.m \
	test/i-93_01.bmf test/i-93_02.bmf test/i-93_03.xad test/i-93_04.bmf \
	test/i-93_05.xad test/i-93_06.xad test/i-93_07.bmf test/i-93_08.xad \
	test/i-93_09.xad test/i-93_10.xad test/i-93_11.xad test/i-93_12.xad \
	test/i-93_13.xad test/i-93_14.xad test/i-93_15.bmf test/i-93_16.xad \
	test/i-93_17.xad test/i-93_18.xad \
	test/i-94.rix \
	test/i-95_1.ctm test/i-95_2.ctm \
	test/i-96_1.rol	test/i-96_2.rol	test/i-96_3.rol \
	test/i-98.sng \
	test/i-99.dro \
	test/i-100_01.xad test/i-100_02.xad test/i-100_03.dro \
	test/i-100_04.dro test/i-100_05.raw test/i-100_06.raw \
	test/i-100_07.dro test/i-100_08.dro test/i-100_09.adlib \
	test/i-100_10.mkj test/i-100_11.mkj test/i-100_12.cmf \
	test/i-100_13.cmf \
	test/i-101_1.d00 test/i-101_2.d00 \
	test/i-102_001.dfm test/i-102_002.dfm test/i-102_003.dfm \
	test/i-102_004.dfm test/i-102_005.dfm test/i-102_006.dfm \
	test/i-102_007.dfm test/i-102_008.dfm test/i-102_009.dfm \
	test/i-102_010.dfm test/i-102_011.dfm test/i-102_012.dfm \
	test/i-102_013.dfm test/i-102_014.dfm test/i-102_015.dfm \
	test/i-102_016.dfm test/i-102_017.dfm test/i-102_018.dfm \
	test/i-102_019.dfm test/i-102_020.dfm test/i-102_021.dfm \
	test/i-102_022.dfm test/i-102_023.dfm test/i-102_024.dfm \
	test/i-102_025.dfm test/i-102_026.dfm test/i-102_027.dfm \
	test/i-102_028.dfm test/i-102_029.dfm test/i-102_030.dfm \
	test/i-102_031.dfm test/i-102_032.dfm test/i-102_033.dfm \
	test/i-102_034.dfm test/i-102_035.dfm test/i-102_036.dfm \
	test/i-102_037.dfm test/i-102_038.dfm test/i-102_039.dfm \
	test/i-102_040.dfm test/i-102_041.dfm test/i-102_042.dfm \
	test/i-102_043.dfm test/i-102_044.dfm test/i-102_045.dfm \
	test/i-102_046.dfm test/i-102_047.dfm test/i-102_048.dfm \
	test/i-102_049.dfm test/i-102_050.dfm test/i-102_051.dfm \
	test/i-102_052.dfm test/i-102_053.dfm test/i-102_054.dfm \
	test/i-102_055.dfm test/i-102_056.dfm test/i-102_057.dfm \
	test/i-102_058.dfm test/i-102_059.dfm test/i-102_060.dfm \
	test/i-102_061.dfm test/i-102_062.dfm test/i-102_063.dfm \
	test/i-102_064.dfm test/i-102_065.dfm test/i-102_066.dfm \
	test/i-102_067.dfm test/i-102_068.dfm test/i-102_069.dfm \
	test/i-102_070.dfm test/i-102_071.dfm test/i-102_072.dfm \
	test/i-102_073.dfm test/i-102_074.dfm test/i-102_075.dfm \
	test/i-102_076.dfm test/i-102_077.dfm test/i-102_078.dfm \
	test/i-102_079.dfm test/i-102_080.dfm test/i-102_081.dfm \
	test/i-102_082.dfm test/i-102_083.dfm test/i-102_084.dfm \
	test/i-102_085.dfm test/i-102_086.dfm test/i-102_087.dfm \
	test/i-102_088.dfm test/i-102_089.dfm test/i-102_090.dfm \
	test/i-102_091.dfm test/i-102_092.dfm test/i-102_093.dfm \
	test/i-102_094.dfm test/i-102_095.dfm test/i-102_096.dfm \
	test/i-102_097.dfm test/i-102_098.dfm test/i-102_099.dfm \
	test/i-102_100.dfm test/i-102_101.dfm test/i-102_102.dfm \
	test/i-102_103.dfm test/i-102_104.dfm test/i-102_105.dfm \
	test/i-102_106.dfm test/i-102_107.dfm test/i-102_108.dfm \
	test/i-102_109.dfm test/i-102_110.dfm test/i-102_111.dfm \
	test/i-102_112.dfm test/i-102_113.dfm test/i-102_114.dfm \
	test/i-102_115.dfm test/i-102_116.dfm test/i-102_117.dfm \
	test/i-102_118.dfm test/i-102_119.dfm test/i-102_120.dfm \
	test/i-102_121.dfm test/i-102_122.dfm test/i-102_123.dfm \
	test/i-102_124.dfm test/i-102_125.dfm test/i-102_126.dfm \
	test/i-102_127.dfm test/i-102_128.dfm test/i-102_129.dfm \
	test/i-102_130.dfm test/i-102_131.dfm test/i-102_132.dfm \
	test/i-102_133.dfm test/i-102_134.dfm test/i-102_135.dfm \
	test/i-102_136.dfm test/i-102_137.dfm test/i-102_138.dfm \
	test/i-102_139.dfm test/i-102_140.dfm test/i-102_141.dfm \
	test/i-102_142.dfm test/i-102_143.dfm test/i-102_144.dfm \
	test/i-102_145.dfm test/i-102_146.dfm test/i-102_147.dfm \
	test/i-102_148.dfm test/i-102_149.dfm test/i-102_150.dfm \
	test/i-102_151.dfm \
	test/i-110_001.dtm test/i-110_002.dtm test/i-110_003.dtm \
	test/i-110_004.dtm test/i-110_005.a2m test/i-110_006.a2m \
	test/i-110_007.a2m test/i-110_008.a2m test/i-110_009.a2m \
	test/i-110_010.a2m test/i-110_011.a2m test/i-110_012.cff \
	test/i-110_013.cff test/i-110_014.cff test/i-110_015.cff \
	test/i-110_016.cff test/i-110_017.cff test/i-110_018.cff \
	test/i-110_019.cff test/i-110_020.cff test/i-110_021.cff \
	test/i-110_022.cff test/i-110_023.cff test/i-110_024.cff \
	test/i-110_025.amd test/i-110_026.amd test/i-110_027.amd \
	test/i-110_028.amd test/i-110_029.amd test/i-110_030.xad \
	test/i-110_031.dtm test/i-110_032.dtm test/i-110_033.amd \
	test/i-110_034.xad test/i-110_035.xad test/i-110_036.xad \
	test/i-110_037.xad test/i-110_038.a2m test/i-110_039.dtm \
	test/i-110_040.dtm test/i-110_041.a2m test/i-110_042.a2m \
	test/i-110_043.amd test/i-110_044.a2m test/i-110_045.a2m \
	test/i-110_046.amd test/i-110_047.sng test/i-110_048.cff \
	test/i-110_049.sa2 test/i-110_050.sa2 test/i-110_051.amd \
	test/i-110_052.dtm test/i-110_053.dtm test/i-110_054.dtm \
	test/i-110_055.dtm test/i-110_056.amd test/i-110_057.mtk \
	test/i-110_058.xad test/i-110_059.dtm test/i-110_060.dtm \
	test/i-110_061.dtm test/i-110_062.amd test/i-110_063.dtm \
	test/i-110_064.dtm test/i-110_065.amd test/i-110_066.sng \
	test/i-110_067.sat test/i-110_068.amd test/i-110_069.amd \
	test/i-110_070.xad test/i-110_071.amd test/i-110_072.amd \
	test/i-110_073.a2m test/i-110_074.a2m test/i-110_075.a2m \
	test/i-110_076.a2m test/i-110_077.a2m test/i-110_078.a2m \
	test/i-110_079.sng test/i-110_080.sng test/i-110_081.sa2 \
	test/i-110_082.sa2 test/i-110_083.amd test/i-110_084.amd \
	test/i-110_085.amd test/i-110_086.amd test/i-110_087.amd \
	test/i-110_088.amd test/i-110_089.xad test/i-110_090.xad \
	test/i-110_091.a2m test/i-110_092.cff test/i-110_093.cff \
	test/i-110_094.mtk test/i-110_095.xad test/i-110_096.xad \
	test/i-110_097.cff test/i-110_098.amd test/i-110_099.amd \
	test/i-110_100.dtm test/i-110_101.cff test/i-110_102.cff \
	test/i-110_103.cff test/i-110_104.amd test/i-110_105.cff \
	test/i-110_106.sat test/i-110_107.a2m test/i-110_108.a2m \
	test/i-110_109.amd test/i-110_110.dtm test/i-110_111.dtm \
	test/i-110_112.dtm test/i-110_113.dtm test/i-110_114.dtm \
	test/i-110_115.dtm test/i-110_116.dtm test/i-110_117.dtm \
	test/i-110_118.dtm test/i-110_119.dtm test/i-110_120.dtm \
	test/i-110_121.dtm test/i-110_122.dtm test/i-110_123.xad \
	test/i-110_124.a2m test/i-110_125.dtm test/i-110_126.a2m \
	test/i-110_127.amd test/i-110_128.dtm test/i-110_129.a2m \
	test/i-110_130.amd test/i-110_131.amd test/i-110_132.a2m \
	test/i-110_133.dtm test/i-110_134.sat test/i-110_135.amd \
	test/i-110_136.amd


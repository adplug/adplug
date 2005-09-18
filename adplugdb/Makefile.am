bin_PROGRAMS = adplugdb

adplugdb_SOURCES = adplugdb.cpp getopt.c getopt.h

adplugdb_LDADD = -L../src -ladplug $(libbinio_LIBS)

adplug_data_dir = $(datadir)/adplug

AM_CPPFLAGS = -DADPLUG_DATA_DIR=\"$(adplug_data_dir)\" $(libbinio_CFLAGS)

EXTRA_DIST = adplugdb.dsp Makefile.bt vc6inst.bat

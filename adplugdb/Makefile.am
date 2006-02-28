bin_PROGRAMS = adplugdb

adplugdb_SOURCES = adplugdb.cpp getopt.c getopt.h

AM_LDFLAGS = $(top_builddir)/src/.libs/libadplug.la $(libbinio_LIBS)

adplug_data_dir = $(sharedstatedir)/adplug

AM_CPPFLAGS = -DADPLUG_DATA_DIR=\"$(adplug_data_dir)\" $(libbinio_CFLAGS)

EXTRA_DIST = adplugdb.dsp Makefile.bt vc6inst.bat

bin_PROGRAMS = adplugdb

adplugdb_SOURCES = adplugdb.cpp

adplugdb_LDADD = -L../src -ladplug

adplug_data_dir = $(datadir)/adplug

AM_CPPFLAGS = -DADPLUG_DATA_DIR=\"$(adplug_data_dir)\"

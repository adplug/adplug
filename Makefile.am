SUBDIRS = src

EXTRA_DIST = INSTALL.unix INSTALL.win32 INSTALL.dos Makefile.wat \
	Makefile.bt doc/Hacking.ms doc/Player-Development.ms \
	doc/Protracker.ms

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

AUTOMAKE_OPTIONS = dist-bzip2

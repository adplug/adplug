SUBDIRS = src doc

EXTRA_DIST = INSTALL.unix INSTALL.win32 INSTALL.dos Makefile.wat \
	Makefile.bt

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

AUTOMAKE_OPTIONS = dist-bzip2

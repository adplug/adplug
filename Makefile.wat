# Watcom Makefile Patchwork 1.0
# Copyright (c) 2002, 2003 Simon Peter <dn.tlp@gmx.net>
#
# This (and only this) file is released under the terms and conditions
# of the Nullsoft license:
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

### PRECONFIGURATION ###
CC = wcc386
CXX = wpp386
LIB = wlib
LD = wlink
MAKE = wmake
ZIP = pkzip

CFLAGS =
CXXFLAGS =
CPPFLAGS =
LDFLAGS =
LIBFLAGS = -n -b
MAKEFLAGS = /h $+$(__MAKEOPTS__)$-
ZIPFLAGS = -P

#SUBDIRS =
#OUTPUT =
LIBRARIES =
OBJS =
DIST =
BINARYDIST =
SYSTEM = dos4g

#include_INST =
#lib_INST =
#bin_INST =

debug = no
installfile = install.txt
submakefile = Makefile.bt
configfile = config.mif
#distfile =
#binarydistfile =
distlist = srcdist.lst
binarydistlist = bindist.lst
bindir = .
includedir = .
libdir = .
#includesubdir =
### PRECONFIGURATION END ###

!include $(submakefile)
!include $(configfile)

### LOCAL VARIABLES ###
# All subdirectory-recursive targets
Recursive_Targets = all.recursive clean.recursive install.recursive &
uninstall.recursive

# Automatically distributed files
DIST += $(submakefile) $(installfile)

# Debugging support
!ifeq debug yes
CFLAGS += -d2
CXXFLAGS += -d2
LDFLAGS += debug all
!endif
### LOCAL VARIABLES END ###

### MAIN SECTION ###
.c.obj: .AUTODEPEND
	$(CC) $(CFLAGS) $(CPPFLAGS) $[.

.cpp.obj: .AUTODEPEND
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $[.

.obj.lib:
        $(LIB) $(LIBFLAGS) $@ +$<

.obj.exe:
        $(LD) $(LDFLAGS) N $@ F {$<} LIB {$(LIBRARIES)} SYS $(SYSTEM)

all: all.recursive $(OUTPUT) .symbolic

# Recurse through subdirectories.
$(Recursive_Targets): $(__MAKEFILES__) .symbolic
!ifdef CurrentSubdir
        cd $(CurrentSubdir)
        $(MAKE) /f ..\$[@ $(MAKEFLAGS) MAKEFLAGS=$(MAKEFLAGS) &
                configfile=..\$(configfile) $*
!else
        for %d in ($(SUBDIRS)) do &
                $(MAKE) /f $[@ $(MAKEFLAGS) MAKEFLAGS=$(MAKEFLAGS) &
                        configfile=$(configfile) CurrentSubdir=%d $@
!endif

!ifdef OUTPUT
$(OUTPUT): $(OBJS)
!endif

clean: clean.recursive .symbolic
        for %f in ($(OBJS) $(OUTPUT) $(distlist) $(binarydistlist)) do &
                if exist %f del %f

distclean: clean .symbolic
        for %f in ($(configfile) $(distfile) $(binarydistfile)) do &
                if exist %f del %f

# Install the binaries $(bin_INST) into the directory $(bindir). Install the
# headers $(include_INST) into the directory $(includesubdir), which is
# located in the directory $(includedir). Install the libraries $(lib_INST)
# into the directory $(libdir). If the directories do not exist, they are
# created. If some of the binaries or libraries do not exist, they are also
# created.
install: install.recursive $(OUTPUT) .symbolic
!ifdef include_INST
        if not exist $(includedir) mkdir $(includedir)
!ifdef includesubdir
        if not exist $(includedir)\$(includesubdir) &
                mkdir $(includedir)\$(includesubdir)
        for %f in ($(include_INST)) do copy %f $(includedir)\$(includesubdir)
!else
        for %f in ($(include_INST)) do copy %f $(includedir)
!endif
!endif
!ifdef lib_INST
        if not exist $(libdir) mkdir $(libdir)
        for %f in ($(lib_INST)) do copy %f $(libdir)
!endif
!ifdef bin_INST
        if not exist $(bindir) mkdir $(bindir)
        for %f in ($(bin_INST)) do copy %f $(bindir)
!endif

# Uninstalls previously installed files.
uninstall: uninstall.recursive .symbolic
!ifdef include_INST
!ifdef includesubdir
        for %f in ($(include_INST)) do del $(includedir)\$(includesubdir)\%f
        rmdir $(includedir)\$(includesubdir)
!else
        for %f in ($(include_INST)) do del $(includedir)\%f
!endif
!endif
!ifdef lib_INST
        for %f in ($(lib_INST)) do del $(libdir)\%f
!endif
!ifdef bin_INST
        for %f in ($(bin_INST)) do del $(bindir)\%f
!endif

# Create a source distribution with filename $(distfile), containing the files
# $(DIST), using the program $(ZIP). Special options are in $(ZIPFLAGS).
!ifdef distfile
$(distlist): $(__MAKEFILES__)
        if exist $(distlist) del $(distlist)
        echo $[@ > $(distlist)
        for %f in ($(DIST)) do echo %f >> $(distlist)

$(distfile): $(distlist)
        $(ZIP) $(ZIPFLAGS) $(distfile) @$(distlist)

dist: $(distfile) .symbolic
!endif

# Create a binary distribution with filename $(binarydistfile), containing
# the files $(BINARYDIST), using the program $(ZIP). Special options are in
# $(ZIPFLAGS). If the binaries do not exist, they are created.
!ifdef binarydistfile
$(binarydistlist): $(__MAKEFILES__)
        if exist $(binarydistlist) del $(binarydistlist)
        for %f in ($(BINARYDIST)) do echo %f >> $(binarydistlist)

$(binarydistfile): $(binarydistlist)
        $(ZIP) $(ZIPFLAGS) $(binarydistfile) @$(binarydistlist)

binary-dist: all $(binarydistfile) .symbolic
!endif

### MAIN SECTION END ###

Note
----
This build system is tested and proven to work only with WATCOM C/C++ 11.x!
It may compile with other (older) versions, but i can't guarantee it.

Configuration
-------------
Before you can compile anything, create a file called CONFIG.MIF in the
distribution's base directory. It's best to copy the template below out of
this file and modify it to fit your setup.

In most cases, you just want to change the installation target directories
"bindir", "includedir" and "libdir" to your standard binary, header and
library directories. If you want to install the library manually, you can just
leave the file blank, if you don't want to set any other options.

There are numerous possibilities to configure other aspects of the
distribution using the CONFIG.MIF file. You can redefine any variable of the
distributed Makefile, like any standard Makefile variable (CFLAGS, LDFLAGS and
the like), system type using the SYSTEM variable, or change the whole build
dependency tree.

Building
--------
After configuration, run "wmake" to build the program/library.

Installation
------------
If you like to automatically install the program/library, run "wmake install".

For manual installation, you have to examine the distribution's Makefile or
consult its documentation on what to install where.

Cleaning up
-----------
You can run "wmake clean" after the build (and maybe installation) process to
delete any files created during compilation.

If you like, you can even run "wmake distclean" to clean up any additionally
created files and bring back your source directory into the state, it was
after unpacking the distribution archive.

Uninstallation
--------------
You can automatically delete all installed files by running "wmake uninstall".
Note that any files created after installation are being left on the target.

Debugging
---------
To enable debugging with the WATCOM debugger, add the following line to your
CONFIG.MIF file:

debug = yes

This remakes all source files with source-level debugging enabled and
instructs the linker to include debugging information. Be careful with the
SYSTEM setting in your CONFIG.MIF file, though. Only some extenders are
supported by the WATCOM debugger. Some distributions may have defined a
non-compatible extender by default, which you must override by specifying
your own!

CONFIG.MIF Template
-------------------
# Makefile configuration file
# Lines starting with a '#' are comments.
# Variables in capital letters are distribution-specific. Be careful when
# changing any of these!

# If debug is set to "yes", the compiler and linker are instructed to
# include debug information with the compiled objects and executables.
# Default: no
# This example enables debugging.
#debug = yes

# "bindir" is the target directory for binary (.exe) files.
# Default: current directory
bindir = c:\exe

# "includedir" is the target directory for header (.h) files.
# Default: current directory
includedir = c:\include

# "libdir" is the target directory for library (.lib) files.
# Default: current directory
libdir = c:\lib

# SYSTEM defines the target system.
# Default: (distribution specified)
#SYSTEM =

# CFLAGS defines any parameters that should be passed to the C compiler.
# Default: (distribution specified)
#CFLAGS +=

# CXXFLAGS defines any parameters that should be passed to the C++ compiler.
# Default: (distribution specified)
#CXXFLAGS +=

# CPPFLAGS defines preprocessing directives passed to the C/C++ compilers.
# Default: (distribution specified)
#CPPFLAGS +=

# LDFLAGS defines any parameters passed to the linker.
# Default: (distribution specified)
#LDFLAGS +=

# LIBFLAGS defines any parameters passed to the library manager.
# Default: (distribution specified)
#LIBFLAGS +=

# ZIPFLAGS defines any parameters passed to the archive manager.
# Default: (distribution specified)
#ZIPFLAGS =

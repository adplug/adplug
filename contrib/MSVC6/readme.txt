Configuration
-------------
Before you can compile anything, create a file called config.bat in the
source subdirectory (.\src, that is). It's best to copy the template below
out of this file and modify it to fit your setup.

In most cases, you just want to change the installation target directories
"bindir", "includedir" and "libdir" to your standard binary, header and
library directories. If you want to install the library manually, you can
just leave the file blank. This will prevent any automatic installations.

If you don't have any standard directories yet, you have to create them
first. It's best to put them somewhere under your "My Documents" folder, if
you have one and name them appropriately. For example, a good fit would be
"include" for your headers, "lib" for your libraries and "bin" for your
executables. But any other names are just as well, too.

Remember to tell MSVC about your new directories. This is done through the
"Tools->Options..." menu. There, select the "Directories" tab and fill in
the directory names, using the appropriate listboxes.

Setup
-----
Start MSVC and create a new workspace. Then load the project file
adplug.dsp from the source subdirecotry into it and you're finished.

config.bat Template
-------------------
rem Standard libraries subdirectory
set libdir=C:\My Documents\lib

rem Standard headers subdirectory
set includedir=C:\My Documents\include

rem Standard executables subdirectory
set bindir=C:\My Documents\bin

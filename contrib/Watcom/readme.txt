Installing this build system:
-----------------------------
Before you can use it, you have to move this build system in place. To
do that, you just copy the files 'Makefile.wat' and 'Makefile.bt' into
the base directory of the AdPlug distribution. Then, you copy the
files 'adplugdb.bt', 'src.bt' and 'test.bt' into the subdirectories
of the base directory with the same basename (e.g. 'src.bt' goes to
the 'src' subdirectory). After they are in the subdirectories, you
rename each of them to 'Makefile.bt' and you're set.

Appendix to 'install.txt':
--------------------------
The build instructions of the Watcom build are generic and generated
automatically. They assume that the main Makefile would be called 'Makefile'.
This isn't the case for AdPlug's Watcom build! AdPlug's Watcom Makefile is
called 'Makefile.wat'. Thus, you have to give the commandline option
'/f Makefile.wat' every time you run a 'wmake' command. For example, to
install AdPlug, call wmake like this:

wmake /f Makefile.wat install

Do this similary with any other 'wmake' target.

Prerequisites for Watcom C/C++ builds:
--------------------------------------
AdPlug uses the STL, which doesn't come with Watcom's current compiler by
default. However, there is a free STL implementation that works with
Watcom C/C++ 11.0c and OpenWatcom at http://www.stlport.org, called
STLport.

So, in order to compile AdPlug with Watcom C/C++, you first need to upgrade
your version of the compiler to 11.0c, if you don't have this version already.
A free patch to 11.0c is available at http://www.openwatcom.org. The
OpenWatcom compiler is available there, as well.

After you have applied the patch, you can install the STLport library. There
is more to do than just installing the library, so be sure to also read the
following chapter on how to do this properly.

After you're through all this, you should finally be able to compile
AdPlug... ;)

Installing the STLport library:
-------------------------------
Following is a posting which describes how to install and patch the STLport
library for use with Watcom C/C++ 11.0c.

-----------------------------------------------------------------------------
Subject: Re: stlport
From: Kon Tantos <ksoft1@attglobal.net>
Date: Fri, 03 May 2002 06:23:54 +1000
Newsgroups: openwatcom.users.c_c++

to build STLport with Watcom you need to go through some setup &
patching steps:

Basic procedure to install/use STLPort with Watcom v11c

1 installing STLPort
====================
You can install STLPort to any path you like.
Simply uncompress the compressed archive & copy it to your chosen path.

For this discussion I have installed STLPort 4.5 directly below d:\stl\

2 Setup an STLPATH environmental variable
=========================================
Add the following line to your Autoexec.bat. This will be used to allow
Watcom to find the STLPort include files.

SET STLPATH=d:\stl\STLport-4.5\stlport

3 Reboot your PC
================
This ensures that the STLPATH environmental variable is now part of your
environment. To check this start a DOS command line session and type SET at
the prompt. You should see STLPATH listed.

4 Add the STLPort path to your Watcom project
=============================================
You must have Watcom search for STLPort include files before any others. If
you are using the IDE, click on Option/C++ Compiler Switches and add the
STLPort path to the include directories.

In a typical Watcom project you will have the following in the include
directories textbox:

$(%stlpath);$(%watcom)\h;$(%watcom)\h\nt

Note the use of the stlpath environmental variable, rather than an actual
path. This allows you to use the same project on any PC which has STLPort,
Watcom and the STLPATH environmental variable set.

5 Fixes to get STLPort 4.5.3 to compile with Watcom v11c
========================================================
STLPort will NOT compile 'out of the box' with Watcom v11c. The following
patches fix the most commonly found issues.

stlport\stl_user_config.h
-------------------------
line 45 remove the comment from the line:
# define _STLP_NO_OWN_IOSTREAMS 1

stlport\stl\_string.h
---------------------
line 98/99 replace:
return find_if((_CharT*)_M_first, (_CharT*)_M_last,
_Eq_char_bound<_Traits>(__x)) == (_CharT*)_M_last;
with
return find_if(_M_first, _M_last,
_Eq_char_bound<_Traits>(__x)) == _M_last;

stlport\stl\_tree.c
-------------------
at or about line 37 change from:
# define __iterator__ _Rb_tree_iterator<_Value,_Nonconst_traits<_Value> > # define __size_type__ size_t
to
# define __iterator__ _Rb_tree_iterator<_Value,_Nonconst_traits<_Value> >
# define __size_type__ size_t

-- Regards Kon Tantos ksoft1@attglobal.net or kon.tantos@tafe.nsw.edu.au
-----------------------------------------------------------------------------

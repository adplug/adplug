.\" $Id$
.TL
AdPlug Hacking HOWTO
.AU
Simon Peter <dn.tlp@gmx.net>
.
.NH 1
Introduction

.PP
This document gives some coding guidelines for people wishing to hack around
in AdPlug. Be sure to also read the overall and system-specific INSTALL
files in the distribution's base directory to learn more about the build
systems.

.NH 1
Coding Style

.PP
I do not really enforce any coding style guidelines (i don't adhere to any
style guidelines myself, so... ;) ), but it would be nice if you follow the
overall coding style, used throughout most of AdPlug's source files. If you
don't, that's perfectly okay, too.

.PP
Most of today's "super-intelligent" editors, like MSVC's one or GNU Emacs'
c-mode, have their own idea of what the code has to look like, anyway. And
since most people tend to use these, there's no point in torturing them to
always having to change anything their editor thinks it knows better. ;)

.NH 1
Debug Logging

.PP
AdPlug recently offers centralized debug log management. If you like to
implement debug logging inside your code, please follow these guidelines:

.PP
To implement debug logging,
.I
\%#include "\%debug.h"
.R
in your code (this header is
.B NOT
being installed with the rest of the AdPlug header files into your
standard include directory! It is only available in AdPlug's src/
subdirectory!).

.PP
.I debug.h
is C code, so it is useable from both C and C++ sources. The only
function you have to use is the
.I "\%LogWrite(fmt, ...)"
function.
.I \%LogFile()
is used by AdPlug internally.
.I \%LogWrite()
works exactly like
.I \%printf() ,
instead that it writes to a logfile, rather than on the console.

.SH
Please format your log messages like this:

.IP \[bu]
If your method/function is going to output a lot of debug info (i.e. more
than one line), please put a
.I \%LogWrite()
directly at the beginning of your function, which looks like this:

.DS I
.CW
LogWrite("*** yourclass::yourmethod(param1,param2) ***\\n");
.DE

.PP
And put the following line before every return from your function:

.DS I
.CW
LogWrite("--- yourclass::yourmethod ---\\n");
.DE

.PP
This way, one can easily inspect the logfile and know to which function
every logfile-line belongs. The '***' lines mark the start of a function,
and the '---' lines mark the end.

.PP
Please substitute
.I \%param*
with the corresponding parameter values of your function, if that is
reasonable. For example, it won't help much to log a pointer value -
just put something bogus or helpful there, or noting (i.e.  just a
comma, so the logfile reader knows, to which parameters the other
values correspond). But logging the values of passed ints or strings
could be very helpful.

.IP \[bu]
If your method/function is going to output just one line, format the line
something like this:

.DS I
.CW
LogWrite("yourclass::yourmethod(param1,param2): your message\\n");
.DE

.PP
You don't need the '***' and '---' beginning and end markers then.

.IP \[bu]
For asynchronous threads, there is no way but to prefix any line with the
function name, because these messages will be sprayed throughout the
logfile.

.NH 1
Documentation

.PP
AdPlug's documentation is written in ASCII textfiles and Troff
files. This was my own decision, since i consider TeX too bloated. The
Troff documentation currently uses the -ms macro package, as well as
the tbl preprocessor. Any current Troff implementation should support
that. I use GNU Troff for typesetting, myself.

.PP
If you would like to add to the existing documentation, please use the
writing style, that's already been used here.

.PP
If you need more info about Troff or need the software, it is best to
go to the official GNU Troff homepage at:

.IP \[bu]
.I http://www.gnu.org/software/groff/

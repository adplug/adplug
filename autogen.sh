#!/bin/sh
# autogen.sh -- generate build-time files
# Copyright (C) 2002 Simon Peter <dn.tlp@gmx.net>
#
# This script runs the GNU autotools, in the right order, to generate all
# the files needed at build time. It does additional version checks.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

AUTOMAKE_VERSION=`automake --version | head -n 1`

case $AUTOMAKE_VERSION in
    *1.6*)
	echo "Running aclocal, automake and autoconf..."
	;;
    *)
	echo "Your version of automake is:"
	echo ""
	echo "   $AUTOMAKE_VERSION"
	echo ""
	echo "But automake >= 1.6 is required, you can find it at:"
	echo ""
	echo " Source for automake 1.6"
	echo " - ftp://ftp.gnu.org/gnu/automake/"
	echo ""
	echo " Debian Pakages for automake 1.6 are in unstable"
	echo ""
	exit 0
	;;
esac

aclocal
automake --add-missing
autoconf

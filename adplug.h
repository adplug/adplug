/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999, 2000, 2001 Simon Peter, <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * adplug.h - CAdPlug helper class declaration, by Simon Peter <dn.tlp@gmx.net>
 */

#include <iostream.h>

#include "players/player.h"
#include "opl.h"

class CAdPlug
{
public:
	static CPlayer *factory(char *fn, Copl *opl);
	static CPlayer *factory(istream &f, Copl *opl);

	static unsigned long songlength(CPlayer *p, unsigned int subsong = 0xffff);
	static void seek(CPlayer *p, unsigned long ms);

private:
	static CPlayer *load_sci(istream &f, char *fn, Copl *opl);	// special loader for Sierra SCI file format
	static CPlayer *load_ksm(istream &f, char *fn, Copl *opl);	// special loader for Ken Silverman's music format
};

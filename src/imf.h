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
 * imf.h - IMF Player by Simon Peter (dn.tlp@gmx.net)
 */

#include "player.h"

class CimfPlayer: public CPlayer
{
public:
	CimfPlayer(Copl *newopl)
		: CPlayer(newopl), data(0)
	{ };
	~CimfPlayer()
	{ if(data) delete [] data; };

	bool load(istream &f);
	bool update();
	void rewind(unsigned int subsong);
	float getrefresh()
	{ return timer; };

	std::string gettype()
	{ return std::string("IMF File Format"); };

protected:
	unsigned long pos,size;
	unsigned short del;
	bool songend;
	float rate,timer;

	struct Sdata {
		unsigned char reg,val;
		unsigned short time;
	} *data;

private:
	unsigned long crc32(unsigned char *buf, unsigned long size);
	float getrate(unsigned long crc, unsigned long size);
};

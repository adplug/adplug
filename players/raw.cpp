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
 * raw.c - RAW Player by Simon Peter (dn.tlp@gmx.net)
 *
 * NOTES:
 * OPL3 register writes are ignored (not possible with AdLib).
 */

#include "raw.h"

/*** public methods *************************************/

bool CrawPlayer::load(istream &f)
{
	char id[8];
	unsigned long filesize,fpos;

	// file validation section
	f.read(id,8);
	if(strncmp(id,"RAWADATA",8))
		return false;

	// load section
	fpos = f.tellg(); f.seekg(0,ios::end); filesize = f.tellg(); f.seekg(fpos);	// get filesize
	f.read((char *)&clock,sizeof(clock));	// clock speed
	data = (Tdata *) new char [filesize-10];
	f.read((char *)data,filesize-10);

	length = (filesize-10) / 2;
	rewind(0);
	return true;
}

bool CrawPlayer::update()
{
	if(songend || pos > length)
		return false;

	if(del) {
		del--;
		return !songend;
	}

	do {
		switch(data[pos].command) {
		case 0: del = data[pos].param - 1; break;
		case 2: if(!data[pos].param) {
					speed = *(unsigned short *)&data[++pos];
				} else
					opl3 = data[pos].param - 1;
				break;
		case 0xff: if(data[pos].param == 0xff)
						songend = true;
				   break;
		default: if(!opl3)
					opl->write(data[pos].command,data[pos].param);
				 break;
		}
		pos++;
	} while(data[pos-1].command);

	return !songend;
}

void CrawPlayer::rewind(unsigned int subsong)
{
	pos = del = opl3 = 0; speed = clock; songend = false;
	opl->init(); opl->write(1,32);	// go to OPL2 mode
}

float CrawPlayer::getrefresh()
{
	return 1193180 / (float)(speed ? speed : 0xffff);	// timer oscillator speed / wait register = clock frequency
}

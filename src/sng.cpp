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
 * sng.cpp - SNG Player by Simon Peter (dn.tlp@gmx.net)
 */

#include "sng.h"

bool CsngPlayer::load(istream &f)
{
	// file validation section
	f.read((char *)&header,sizeof(header));
	if(strncmp(header.id,"ObsM",4))
		return false;

	// load section
	data = new Sdata [(header.length / 2) + 1];
	f.read((char *)data,header.length);

	header.length /= 2; header.start /= 2; header.loop /= 2;

	rewind(0);
	return true;
}

bool CsngPlayer::update()
{
	if(header.compressed && del) {
		del--;
		return !songend;
	}

	while(data[pos].reg) {
		if(pos < header.length) {
			opl->write(data[pos].reg,data[pos].val);
			pos++;
		} else {
			songend = true;
			pos = header.loop;
		}
	}

	if(!header.compressed)
		opl->write(data[pos].reg,data[pos].val);

	if(data[pos].val) del = data[pos].val - 1; pos++;
	return !songend;
}

void CsngPlayer::rewind(unsigned int subsong)
{
	pos = header.start; del = header.delay; songend = false;
	opl->init(); opl->write(1,32);	// go to OPL2 mode
}

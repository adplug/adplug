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
 * dfm.cpp - Digital-FM Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include <stdio.h>
#include <string.h>

#include "dfm.h"

bool CdfmLoader::load(istream &f)
{
	unsigned char		tmpins[11],npats,n,note,fx,c,r,param;
	unsigned int		i;
	const unsigned char	convfx[8] = {255,255,17,19,23,24,255,13};

	// file validation
	f.read((char *)&header,6);
	if(strncmp(header.id,"DFM\x1a",4) || header.hiver > 1)
		return false;

	// load
	restartpos = 0; activechan = 0xffff; flags = MOD_FLAGS_STANDARD; bpm = 0;
	for(i=0;i<64*9;i++) trackord[i/9][i%9] = i+1;
	memset(tracks,0,576*64);
	f.read(songinfo,33);
	initspeed = f.get();
	f.read((char *)instname,32*12);
	for(i=0;i<32;i++) {
		f.read((char *)tmpins,11);
		inst[i].data[0] = tmpins[10];
		inst[i].data[1] = tmpins[0];
		inst[i].data[2] = tmpins[1];
		inst[i].data[3] = tmpins[4];
		inst[i].data[4] = tmpins[5];
		inst[i].data[5] = tmpins[6];
		inst[i].data[6] = tmpins[7];
		inst[i].data[7] = tmpins[8];
		inst[i].data[8] = tmpins[9];
		inst[i].data[9] = tmpins[2];
		inst[i].data[10] = tmpins[3];
	}
	f.read((char *)order,128);
	for(i=0;i<128 && order[i] != 128;i++) ;
	length = i;
	npats = f.get();
	for(i=0;i<npats;i++) {
		n = f.get();
		for(r=0;r<64;r++)
			for(c=0;c<9;c++) {
				note = f.get();
				if((note & 15) == 15)
					tracks[n*9+c][r].note = 127;	// key off
				else
					tracks[n*9+c][r].note = ((note & 127) >> 4) * 12 + (note & 15);
				if(note & 128) {	// additional effect byte
					fx = f.get();
					if(fx >> 5 == 1)
						tracks[n*9+c][r].inst = (fx & 31) + 1;
					else {
						tracks[n*9+c][r].command = convfx[fx >> 5];
						if(tracks[n*9+c][r].command == 17) {	// set volume
							param = fx & 31;
							param = 63 - param * 2;
							tracks[n*9+c][r].param1 = param >> 4;
							tracks[n*9+c][r].param2 = param & 15;
						} else {
							tracks[n*9+c][r].param1 = (fx & 31) >> 4;
							tracks[n*9+c][r].param2 = fx & 15;
						}
					}
				}

			}
	}

	rewind(0);
	return true;
}

std::string CdfmLoader::gettype()
{
	char tmpstr[20];

	sprintf(tmpstr,"Digital-FM %d.%d",header.hiver,header.lover);
	return std::string(tmpstr);
}

float CdfmLoader::getrefresh()
{
	return 125.0f;
}

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
 * sa2.cpp - SAdT2 Loader by Simon Peter (dn.tlp@gmx.net)
 *           SAdT Loader by Mamiya (mamiya@users.sourceforge.net)
 */

#include <stdio.h>

#include "sa2.h"

bool Csa2Loader::load(istream &f)
{
	struct {
		unsigned char data[11],arpstart,arpspeed,arppos,arpspdcnt;
	} insts;
	unsigned char buf;
	int i,j, k, notedis = 0;
	const unsigned char convfx[16] = {0,1,2,3,4,5,6,255,8,255,10,11,12,13,255,15};
	unsigned sat_type;
	enum SAT_TYPE {
		HAS_ARPEGIOLIST = (1 << 7),
		HAS_V7PATTERNS = (1 << 6),
		HAS_ACTIVECHANNELS = (1 << 5),
		HAS_TRACKORDER = (1 << 4),
		HAS_ARPEGIO = (1 << 3),
		HAS_OLDBPM = (1 << 2),
		HAS_OLDPATTERNS = (1 << 1),
		HAS_UNKNOWN127 = (1 << 0)
	};

	// file validation section
	f.read((char *)&header,sizeof(sa2header));
	if(strncmp(header.sadt,"SAdT",4))
		return false;
	switch(header.version) {
	case 1:
		notedis = +0x18;
		sat_type = HAS_UNKNOWN127 | HAS_OLDPATTERNS | HAS_OLDBPM;
		break;
	case 2:
		notedis = +0x18;
		sat_type = HAS_OLDPATTERNS | HAS_OLDBPM;
		break;
	case 3:
		notedis = +0x0c;
		sat_type = HAS_OLDPATTERNS | HAS_OLDBPM;
		break;
	case 4:
		notedis = +0x0c;
		sat_type = HAS_ARPEGIO | HAS_OLDPATTERNS | HAS_OLDBPM;
		break;
	case 5:
		notedis = +0x0c;
		sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_OLDPATTERNS | HAS_OLDBPM;
		break;
	case 6:
		sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_OLDPATTERNS | HAS_OLDBPM;
		break;
	case 7:
		sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_V7PATTERNS;
		break;
	case 8:
		sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_TRACKORDER;
		break;
	case 9:
		sat_type = HAS_ARPEGIO | HAS_ARPEGIOLIST | HAS_TRACKORDER | HAS_ACTIVECHANNELS;
		break;
	default:	/* unknown */
		return false;
	}

	// load section
	for(i = 0; i < 31; i++) {
		if(sat_type & HAS_ARPEGIO) {
			f.read((char *)&insts,15);			// instruments
			inst[i].arpstart = insts.arpstart;
			inst[i].arpspeed = insts.arpspeed;
			inst[i].arppos = insts.arppos;
			inst[i].arpspdcnt = insts.arpspdcnt;
		} else {
			f.read((char *)&insts,11);			// instruments
			inst[i].arpstart = 0;
			inst[i].arpspeed = 0;
			inst[i].arppos = 0;
			inst[i].arpspdcnt = 0;
		}
		for(j=0;j<11;j++)
			inst[i].data[j] = insts.data[j];
		inst[i].misc = 0;
		inst[i].slide = 0;
	}
	f.read((char *)instname,29*17);				// instrument names
	f.ignore(3);						// dummy bytes
	f.read((char *)order,128);				// pattern orders
	if (sat_type & HAS_UNKNOWN127) f.ignore(127);

	// infos
	f.read((char *)&nop,2); f.read((char *)&length,1); f.read((char *)&restartpos,1);

	// bpm
	f.read((char *)&bpm,2);
	if(sat_type & HAS_OLDBPM) {
		bpm = bpm * 125 / 50;					// cps -> bpm
	}

	if(sat_type & HAS_ARPEGIOLIST) {
		f.read((char *)arplist,sizeof(arplist));	// arpeggio list
		f.read((char *)arpcmd,sizeof(arpcmd));		// arpeggio commands
	}

	for(i=0;i<64;i++) {					// track orders
		for(j=0;j<9;j++) {
			if(sat_type & HAS_TRACKORDER)
				f.read((char *)&trackord[i][j],1);
			else
			{
				trackord[i][j] = i * 9 + j;
			}
		}
	}

	if(sat_type & HAS_ACTIVECHANNELS)
		f.read((char *)&activechan,2);			// active channels
	else
		activechan = 0xffff;

	// track data
	if(sat_type & HAS_OLDPATTERNS) {
		i = 0;
		while(f.peek() != EOF) {
			for(j=0;j<64;j++) {
				for(k=0;k<9;k++) {
					buf = f.get();
					tracks[i+k][j].note = buf ? (buf + notedis) : 0;
					tracks[i+k][j].inst = f.get();
					tracks[i+k][j].command = convfx[f.get() & 0xf];
					tracks[i+k][j].param1 = f.get();
					tracks[i+k][j].param2 = f.get();
				}
			}
			i+=9;
		}
	} else if(sat_type & HAS_V7PATTERNS) {
		i = 0;
		while(f.peek() != EOF) {
			for(j=0;j<64;j++) {
				for(k=0;k<9;k++) {
					buf = f.get();
					tracks[i+k][j].note = buf >> 1;
					tracks[i+k][j].inst = (buf & 1) << 4;
					buf = f.get();
					tracks[i+k][j].inst += buf >> 4;
					tracks[i+k][j].command = convfx[buf & 0x0f];
					buf = f.get();
					tracks[i+k][j].param1 = buf >> 4;
					tracks[i+k][j].param2 = buf & 0x0f;
				}
			}
			i+=9;
		}
	} else {
		i = 0;
		while(f.peek() != EOF) {
			for(j=0;j<64;j++) {
				buf = f.get();
				tracks[i][j].note = buf >> 1;
				tracks[i][j].inst = (buf & 1) << 4;
				buf = f.get();
				tracks[i][j].inst += buf >> 4;
				tracks[i][j].command = convfx[buf & 0x0f];
				buf = f.get();
				tracks[i][j].param1 = buf >> 4;
				tracks[i][j].param2 = buf & 0x0f;
			}
			i++;
		}
	}

	// fix instrument names
	for(i=0;i<29;i++)
		for(j=0;j<17;j++)
			if(!instname[i][j])
				instname[i][j] = ' ';

	rewind(0);		// rewind module
	return true;
}

std::string Csa2Loader::gettype()
{
	char tmpstr[40];

	sprintf(tmpstr,"Surprise! Adlib Tracker 2 (version %d)",header.version);
	return std::string(tmpstr);
}

std::string Csa2Loader::gettitle()
{
	char bufinst[29*17],buf[18];
	int i,ptr;

	// parse instrument names for song name
	memset(bufinst,'\0',29*17);
	for(i=0;i<29;i++) {
		buf[16] = ' '; buf[17] = '\0';
		memcpy(buf,instname[i]+1,16);
		for(ptr=16;ptr>0;ptr--)
			if(buf[ptr] == ' ')
				buf[ptr] = '\0';
			else {
				if(ptr<16)
					buf[ptr+1] = ' ';
				break;
			}
		strcat(bufinst,buf);
	}

	if(strchr(bufinst,'"'))
		return std::string(bufinst,strchr(bufinst,'"')-bufinst+1,strrchr(bufinst,'"')-strchr(bufinst,'"')-1);
	else
		return std::string();
}

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * mkj.cpp - MKJamz Player, by Simon Peter <dn.tlp@gmx.net>
 */

#include "mkj.h"

CPlayer *CmkjPlayer::factory(Copl *newopl)
{
  return new CmkjPlayer(newopl);
}

bool CmkjPlayer::load(const std::string &filename, const CFileProvider &fp)
{
        binistream *f = fp.open(filename); if(!f) return false;
	char	id[6];
	float	ver;
	int	i, j;
	short	inst[8];

	// file validation
	f->readString(id, 6);
	if(strncmp(id,"MKJamz",6)) { fp.close(f); return false; }
	ver = f->readFloat(binio::Single);
	if(ver > 1.12) { fp.close(f); return false; }

	// load
	maxchannel = f->readInt(2);
	opl->init(); opl->write(1, 32);
	for(i = 0; i < maxchannel; i++) {
	        for(j = 0; j < 8; j++) inst[j] = f->readInt(2);
		opl->write(0x20+op_table[i],inst[4]);
		opl->write(0x23+op_table[i],inst[0]);
		opl->write(0x40+op_table[i],inst[5]);
		opl->write(0x43+op_table[i],inst[1]);
		opl->write(0x60+op_table[i],inst[6]);
		opl->write(0x63+op_table[i],inst[2]);
		opl->write(0x80+op_table[i],inst[7]);
		opl->write(0x83+op_table[i],inst[3]);
	}
	maxnotes = f->readInt(2);
	songbuf = new short [(maxchannel+1)*maxnotes];
	for(i = 0; i < maxchannel; i++) channel[i].defined = f->readInt(2);
	for(i = 0; i < (maxchannel + 1) * maxnotes; i++)
	  songbuf[i] = f->readInt(2);

	fp.close(f);
	rewind(0);
	return true;
}

bool CmkjPlayer::update()
{
	int		c,i;
	short	note;

	for(c=0;c<maxchannel;c++) {
		if(!channel[c].defined)
			continue;

		if(channel[c].isdone) {
			channel[c].pstat = channel[c].speed;
			channel[c].songptr += maxchannel;
			channel[c].isdone = false;
			channel[c].isplaying = false;
		}

		note = songbuf[channel[c].songptr + c];
		if(!channel[c].isplaying && note != 0) {
			channel[c].pstat = channel[c].speed;
			switch(note) {
			case 68: opl->write(0xa0 + c,0x81); opl->write(0xb0 + c,0x21 + 4 * channel[c].octave); break;
			case 69: opl->write(0xa0 + c,0xb0); opl->write(0xb0 + c,0x21 + 4 * channel[c].octave); break;
			case 70: opl->write(0xa0 + c,0xca); opl->write(0xb0 + c,0x21 + 4 * channel[c].octave); break;
			case 71: opl->write(0xa0 + c,0x2); opl->write(0xb0 + c,0x22 + 4 * channel[c].octave); break;
			case 65: opl->write(0xa0 + c,0x41); opl->write(0xb0 + c,0x22 + 4 * channel[c].octave); break;
			case 66: opl->write(0xa0 + c,0x87); opl->write(0xb0 + c,0x22 + 4 * channel[c].octave); break;
			case 67: opl->write(0xa0 + c,0xae); opl->write(0xb0 + c,0x22 + 4 * channel[c].octave); break;
			case 17: opl->write(0xa0 + c,0x6b); opl->write(0xb0 + c,0x21 + 4 * channel[c].octave); break;
			case 18: opl->write(0xa0 + c,0x98); opl->write(0xb0 + c,0x21 + 4 * channel[c].octave); break;
			case 20: opl->write(0xa0 + c,0xe5); opl->write(0xb0 + c,0x21 + 4 * channel[c].octave); break;
			case 21: opl->write(0xa0 + c,0x20); opl->write(0xb0 + c,0x22 + 4 * channel[c].octave); break;
			case 15: opl->write(0xa0 + c,0x63); opl->write(0xb0 + c,0x22 + 4 * channel[c].octave); break;
			case 255:	// delay
				channel[c].pstat = songbuf[channel[c].songptr + c + maxchannel] + 2;
				channel[c].isplaying = true;
				channel[c].flag = true;
				break;
			case 254:	// set octave
				channel[c].octave = songbuf[channel[c].songptr + c + maxchannel];
				channel[c].songptr += maxchannel;
				break;
			case 253:	// set speed
				channel[c].speed = songbuf[channel[c].songptr + c + maxchannel] - 1;
				channel[c].songptr += maxchannel;
				break;
			case 252:	// set waveform
				channel[c].waveform = songbuf[channel[c].songptr + c + maxchannel] - 300;
				channel[c].songptr += maxchannel;
				if(c > 2)
					opl->write(0xe0 + c + (c+6),channel[c].waveform);
				else
					opl->write(0xe0 + c,channel[c].waveform);
				break;
			case 251:	// song end
				for(i=0;i<maxchannel;i++) {
					channel[i].songptr = 0;
					channel[i].isplaying = false;
					channel[i].isdone = false;
				}
				songend = true;
				return false;
			}
		}

		if(channel[c].isplaying)
			if(channel[c].pstat)
				channel[c].pstat--;
			else {
				opl->write(0xb0 + c,0);
				channel[c].isdone = true;
				if(channel[c].flag) {
					channel[c].songptr += maxchannel;
					channel[c].flag = false;
				}
			}

		if(channel[c].songptr > maxchannel)
			if(/* songbuf[channel[c].songptr + c] */ note != 0 && songbuf[channel[c].songptr - maxchannel + c] < 250)
				if(!channel[c].isplaying)
					channel[c].isplaying = true;
	}

	for(c=0;c<maxchannel;c++)
		if(!channel[c].isplaying) {
			channel[c].songptr += maxchannel;
			if(channel[c].songptr >= ((maxchannel+1)*maxnotes) / maxchannel)
				channel[c].songptr = 0;
		}

	return !songend;
}

void CmkjPlayer::rewind(int subsong)
{
	int i;

	for(i=0;i<maxchannel;i++) {
		channel[i].flag = false;
		channel[i].isdone = false;
		channel[i].isplaying = false;
		channel[i].pstat = 0;
		channel[i].speed = 0;
		channel[i].waveform = 0;
		channel[i].songptr = 0;
		channel[i].octave = 4;
	}
	songend = false;
}

float CmkjPlayer::getrefresh()
{
	return 100.0f;
}

/*
 * mkj.cpp - MKJamz Player, by Simon Peter (dn.tlp@gmx.net)
 */

#include "mkj.h"

bool CmkjPlayer::load(istream &f)
{
	char			id[6];
	float			ver;
	unsigned int	i;
	short			inst[8];

	// file validation
	f.read(id,6);
	if(strncmp(id,"MKJamz",6)) return false;
	f.read((char *)&ver,sizeof(ver));
	if(ver > 1.12) return false;

	// load
	f.read((char *)&maxchannel,2);
	opl->init(); opl->write(1,32);
	for(i=0;i<maxchannel;i++) {
		f.read((char *)inst,8*2);
		opl->write(0x20+op_table[i],inst[4]);
		opl->write(0x23+op_table[i],inst[0]);
		opl->write(0x40+op_table[i],inst[5]);
		opl->write(0x43+op_table[i],inst[1]);
		opl->write(0x60+op_table[i],inst[6]);
		opl->write(0x63+op_table[i],inst[2]);
		opl->write(0x80+op_table[i],inst[7]);
		opl->write(0x83+op_table[i],inst[3]);
	}
	f.read((char *)&maxnotes,2);
	songbuf = new short [(maxchannel+1)*maxnotes];
	for(i=0;i<maxchannel;i++)
		f.read((char *)&channel[i].defined,2);
	f.read((char *)songbuf,(maxchannel+1)*maxnotes*2);

	rewind(0);
	return true;
}

bool CmkjPlayer::update()
{
	unsigned int	c,i;
	short			note;

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

void CmkjPlayer::rewind(unsigned int subsong)
{
	unsigned int i;

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

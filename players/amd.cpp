/*
 * amd.cpp - AMD Player by Simon Peter (dn.tlp@gmx.net)
 */

#include "amd.h"

bool CamdLoader::load(istream &f)
{
	struct {
		char id[9];
		unsigned char version;
	} header;
	int i,j,t;
	unsigned char buf,buf2,buf3;
	const unsigned char convfx[10] = {0,1,2,9,17,11,13,18,3,14};

	// file validation section
	f.seekg(0,ios::end);
	if(f.tellg() < 1062)
		return false;
	f.seekg(1062);
	f.read((char *)&header,sizeof(header));
	if(strncmp(header.id,"<o\xefQU\xeeRoR",9))
		return false;

	// load section
	version = header.version;
	memset(inst,0,sizeof(inst));
	f.seekg(0);
	f.read(songname,sizeof(songname));
	f.read(author,sizeof(author));
	for(i=0;i<26;i++) {
		f.read(instname[i],23);
		f.read(inst[i].data,11);
	}
	length = f.get();
	nop = f.get() + 1;	// convert to 16bit
	f.read(order,128);
	f.read((char *)&header,sizeof(header));
	if(header.version == 0x10) {	// unpacked module
		for(i=0;i<64*9;i++)
			trackord[i/9][i%9] = i+1;
		t = 0;
		while(f.peek() != EOF) {
			for(j=0;j<64;j++)
				for(i=t;i<t+9;i++) {
					buf = f.get();
					tracks[i][j].param2 = (buf&127) % 10;
					tracks[i][j].param1 = (buf&127) / 10;
					buf = f.get();
					tracks[i][j].inst = buf >> 4;
					tracks[i][j].command = buf & 0x0f;
					buf = f.get();
					if(buf >> 4)	// fix bug in AMD save routine
						tracks[i][j].note = ((buf & 14) >> 1) * 12 + (buf >> 4);
					else
						tracks[i][j].note = 0;
					tracks[i][j].inst += (buf & 1) << 4;
				}
			t += 9;
		}
	} else {						// packed module
		for(i=0;i<nop;i++)
			for(j=0;j<9;j++) {
				f.read((char *)&trackord[i][j],2);
				trackord[i][j]++;
			}
		i = 0;
		f.ignore(2);
		while(f.peek() != EOF) {
			f.read((char *)&i,2);
			j = 0;
			do {
				buf = f.get();
				if(buf & 128) {
					for(t=j;t<j+(buf & 127) && t < 64;t++) {
						tracks[i][t].command = 0;
						tracks[i][t].inst = 0;
						tracks[i][t].note = 0;
						tracks[i][t].param1 = 0;
						tracks[i][t].param2 = 0;
					}
					j += buf & 127;
					continue;
				}
				tracks[i][j].param2 = buf % 10;
				tracks[i][j].param1 = buf / 10;
				buf = f.get();
				tracks[i][j].inst = buf >> 4;
				tracks[i][j].command = buf & 0x0f;
				buf = f.get();
				if(buf >> 4)	// fix bug in AMD save routine
					tracks[i][j].note = ((buf & 14) >> 1) * 12 + (buf >> 4);
				else
					tracks[i][j].note = 0;
				tracks[i][j].inst += (buf & 1) << 4;
				j++;
			} while(j<64);
		}
	}

	// convert to protracker replay data
	bpm = 50; restartpos = 0; activechan = 0xffff; flags = MOD_FLAGS_DECIMAL;
	for(i=0;i<26;i++) {	// convert instruments
		buf = inst[i].data[0];
		buf2 = inst[i].data[1];
		inst[i].data[0] = inst[i].data[10];
		inst[i].data[1] = buf;
		buf = inst[i].data[2];
		inst[i].data[2] = inst[i].data[5];
		buf3 = inst[i].data[3];
		inst[i].data[3] = buf;
		buf = inst[i].data[4];
		inst[i].data[4] = inst[i].data[7];
		inst[i].data[5] = buf3;
		buf3 = inst[i].data[6];
		inst[i].data[6] = inst[i].data[8];
		inst[i].data[7] = buf;
		inst[i].data[8] = inst[i].data[9];
		inst[i].data[9] = buf2;
		inst[i].data[10] = buf3;
		for(j=0;j<23;j++)	// convert names
			if(instname[i][j] == '\xff')
				instname[i][j] = '\x20';
	}
	for(i=0;i<nop*9;i++)	// convert patterns
		for(j=0;j<64;j++) {
			tracks[i][j].command = convfx[tracks[i][j].command];
			if(tracks[i][j].command == 14) {
				if(tracks[i][j].param1 == 2) {
					tracks[i][j].command = 10;
					tracks[i][j].param1 = tracks[i][j].param2;
					tracks[i][j].param2 = 0;
				}
				if(tracks[i][j].param1 == 3) {
					tracks[i][j].command = 10;
					tracks[i][j].param1 = 0;
				}
			}
		}
	rewind(0);
	return true;
}

float CamdLoader::getrefresh()
{
	if(tempo)
		return (float) (tempo);
	else
		return 18.2f;
}

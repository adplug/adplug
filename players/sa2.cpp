/*
 * sa2.cpp - SAdT2 Loader by Simon Peter (dn.tlp@gmx.net)
 *
 * NOTES:
 * SAdT2 version 7 files are unimplemented (i don't have any).
 */

#include <stdio.h>

#include "sa2.h"

bool Csa2Loader::load(istream &f)
{
	struct {
		unsigned char data[11],arpstart,arpspeed,arppos,arpspdcnt;
	} insts[31];
	unsigned char buf;
	int i,j;
	const unsigned char convfx[16] = {0,1,2,3,4,5,6,255,8,255,10,11,12,13,255,15};

	// file validation section
	f.read((char *)&header,sizeof(sa2header));
	if(strncmp(header.sadt,"SAdT",4) || (header.version != 9 && header.version != 8))
		return false;

	// load section
	f.read((char *)insts,31*15);				// instruments
	f.read((char *)instname,29*17);				// instrument names
	f.ignore(3);								// dummy bytes
	f.read(order,sizeof(order));				// pattern orders
	f.read((char *)&nop,2); f.read(&length,1); f.read(&restartpos,1); f.read((char *)&bpm,2);	// infos
	f.read(arplist,sizeof(arplist));			// arpeggio list
	f.read(arpcmd,sizeof(arpcmd));				// arpeggio commands
	for(i=0;i<64*9;i++)							// track orders
		f.read((char *)&trackord[i/9][i%9],1);
	if(header.version == 9)
		f.read((char *)&activechan,2);			// active channels
	else
		activechan = 0xffff;	// v8 files have always all channels active

	// track data
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

	// convert instruments
	for(i=0;i<31;i++) {
		for(j=0;j<11;j++)
			inst[i].data[j] = insts[i].data[j];
		inst[i].arpstart = insts[i].arpstart;
		inst[i].arpspeed = insts[i].arpspeed;
		inst[i].arppos = insts[i].arppos;
		inst[i].arpspdcnt = insts[i].arpspdcnt;
		inst[i].misc = 0;
		inst[i].slide = 0;
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

/*
 * rad.cpp - RAD Loader by Simon Peter (dn.tlp@gmx.net)
 *
 * BUGS:
 * some volumes are dropped out
 */

#include "rad.h"

bool CradLoader::load(istream &f)
{
	char id[16];
	unsigned char buf,ch,c,b,inp;
	char bufstr[2] = "\0";
	int i,j;
	unsigned short patofs[32];
	const unsigned char convfx[16] = {255,1,2,3,255,5,255,255,255,255,20,255,17,0xd,255,19};

	// file validation section
	f.read(id,16); version = f.get();
	if(strncmp(id,"RAD by REALiTY!!",16) || version != 0x10)
		return false;

	// load section
	radflags = f.get();
	if(radflags & 128) {	// description
		memset(desc,0,80*22);
		while((buf = f.get()))
			if(buf == 1)
				strcat(desc,"\n");
			else
				if(buf >= 2 && buf <= 0x1f)
					for(i=0;i<buf;i++)
						strcat(desc," ");
				else {
					*bufstr = buf;
					strcat(desc,bufstr);
				}
	}
	while((buf = f.get())) {	// instruments
		buf--;
		inst[buf].data[2] = f.get(); inst[buf].data[1] = f.get();
		inst[buf].data[10] = f.get(); inst[buf].data[9] = f.get();
		inst[buf].data[4] = f.get(); inst[buf].data[3] = f.get();
		inst[buf].data[6] = f.get(); inst[buf].data[5] = f.get();
		inst[buf].data[0] = f.get();
		inst[buf].data[8] = f.get(); inst[buf].data[7] = f.get();
	}
	length = f.get();
	f.read(order,length);	// orderlist
	f.read((char *)patofs,32*2);	// pattern offset table
	for(i=0;i<64*9;i++)		// patterns
		trackord[i/9][i%9] = i+1;
	memset(tracks,0,576*64);
	for(i=0;i<32;i++)
		if(patofs[i]) {
			f.seekg(patofs[i]);
			do {
				buf = f.get(); b = buf & 127;
				do {
					ch = f.get(); c = ch & 127;
					inp = f.get();
					tracks[i*9+c][b].note = inp & 127;
					tracks[i*9+c][b].inst = (inp & 128) >> 3;
					inp = f.get();
					tracks[i*9+c][b].inst += inp >> 4;
					tracks[i*9+c][b].command = inp & 15;
					if(inp & 15) {
						inp = f.get();
						tracks[i*9+c][b].param1 = inp / 10;
						tracks[i*9+c][b].param2 = inp % 10;
					}
				} while(!(ch & 128));
			} while(!(buf & 128));
		} else
			memset(trackord[i],0,9*2);

	// convert replay data
	for(i=0;i<32*9;i++)	// convert patterns
		for(j=0;j<64;j++) {
			if(tracks[i][j].note == 15)
				tracks[i][j].note = 127;
			if(tracks[i][j].note > 16 && tracks[i][j].note < 127)
				tracks[i][j].note -= 4 * (tracks[i][j].note >> 4);
			if(tracks[i][j].note && tracks[i][j].note < 126)
				tracks[i][j].note++;
			tracks[i][j].command = convfx[tracks[i][j].command];
		}
	restartpos = 0; activechan = 0xffff; initspeed = radflags & 31;
	bpm = radflags & 64 ? 0 : 50; flags = MOD_FLAGS_DECIMAL;

	rewind(0);
	return true;
}

float CradLoader::getrefresh()
{
	if(tempo)
		return (float) (tempo);
	else
		return 18.2f;
}

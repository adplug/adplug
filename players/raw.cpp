/*
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

	if(del > 1) {
		del--;
		return !songend;
	}

	do {
		switch(data[pos].command) {
		case 0: del = data[pos].param; break;
		case 2: if(!data[pos].param) {
					speed = *(unsigned short *) &data[++pos];
					if(!speed)
						speed = 0xffff;
				} else
					opl3 = data[pos].param - 1;
				break;
		case 0xff: if(data[pos].param == 0xff)
						songend = 1;
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
	if(!clock) clock = 0xffff;
	pos = 0; del = 0; speed = clock; songend = 0; opl3 = 0;
	opl->init(); opl->write(1,32);	// go to OPL2 mode
}

float CrawPlayer::getrefresh()
{
	return 1193180 / (float) (speed+1);	// timer oscillator speed / (wait register+1) = clock frequency
}

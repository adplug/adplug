/*
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

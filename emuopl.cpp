/*
 * emuopl.cpp - Emulated OPL, by Simon Peter (dn.tlp@gmx.net)
 */

#include "emuopl.h"

void CEmuopl::update(short *buf, int samples)
{
	int i;

	if(use16bit) {
		YM3812UpdateOne(opl,buf,samples);

		if(stereo)
			for(i=samples-1;i>=0;i--) {
				buf[i*2] = buf[i];
				buf[i*2+1] = buf[i];
			}
	} else {
		short *tempbuf = new short[stereo ? samples*2 : samples];
		int i;

		YM3812UpdateOne(opl,tempbuf,samples);

		if(stereo)
			for(i=samples-1;i>=0;i--) {
				tempbuf[i*2] = tempbuf[i];
				tempbuf[i*2+1] = tempbuf[i];
			}

		for(i=0;i<(stereo ? samples*2 : samples);i++)
			((char *)buf)[i] = (tempbuf[i] >> 8) ^ 0x80;

		delete [] tempbuf;
	}
}

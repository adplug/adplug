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
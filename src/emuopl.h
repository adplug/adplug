/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2004 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * emuopl.h - Emulated OPL, by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_EMUOPL
#define H_ADPLUG_EMUOPL

#include "opl.h"
extern "C" {
#include "fmopl.h"
#include "ymf262.h"
}

class CEmuopl: public Copl_2
{
public:
	CEmuopl(int rate, bool bit16, bool usestereo);	// rate = sample rate
	virtual ~CEmuopl();

	void update(short *buf, int samples);	// fill buffer

	// template methods
	void write(int reg, int val);
	void init();

	//for Copl_2 interface
	void setChip(int chip);
	void setMode(int mode);

private:
	bool	use16bit,stereo;
	FM_OPL	*opl[2];				// holds opl2 emulator data
	YMF262::Class *opl3;			//opl3 emulator

	short *mixbuf0;
	short *mixbuf1;
	int mixbufSamples;
	
	//specified by Copl_2 interface
	int currChip; 
	int currMode;
	


};

#endif

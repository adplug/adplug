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
 * kemuopl.h - Emulated OPL using Ken Silverman's emulator, by Simon Peter (dn.tlp@gmx.net)
 */

#include "opl.h"
extern "C" {
#include "adlibemu.h"
}

class CKemuopl: public Copl
{
public:
	CKemuopl(int rate, bool bit16, bool usestereo)
		: use16bit(bit16), stereo(usestereo)
	{
		adlib=adlibinit(rate,usestereo ? 2 : 1,bit16 ? 2 : 1);
	};

	~CKemuopl()
	{
		adlibshutdown(adlib);
	};

	void update(short *buf, int samples)
	{
		adlibgetsample(adlib,buf,samples);
	};

	// template methods
	void write(int reg, int val)
	{
		adlibwrite(adlib,reg,val);
	};
	void init()
	{
		adlibreset(adlib);
	};

private:
	bool use16bit,stereo;
	ADLIB_STATE *adlib;
};

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
 * opl.h - OPL base class declaration, by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_ADPLUG_OPL
#define H_ADPLUG_OPL

class Copl
{
public:
	virtual void write(int reg, int val) = 0;		// combined register select + data write
	virtual void init(void) = 0;				// reinitialize OPL chip

	virtual void update(short *buf, int samples) {};	// Emulation only: fill buffer

	virtual void foo()
};

//a newer interface for OPL renderers--
//provides a mechanism for specifying the 
//chipset to be used for emulation, as well as
//a destination chip for calls to Copl's write()
class Copl_2 : public Copl
{
public:
	//0 = low register set or opl2#0
	//1 = high register set or opl2#1
	//default chip should be 0 for compatibility with Copl interface
	virtual void setChip(int chip) =0;

	//0 = opl2
	//1 = opl3
	//2 = dual-opl2
	//default mode should be 0 for compatibility with Copl interface
	virtual void setMode(int mode) =0;
};

#endif

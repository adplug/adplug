/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999, 2000, 2001, 2002 Simon Peter, <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/*
  dtm.cpp - DTM loader by Riven the Mage <riven@ok.ru>
*/

//#include <windows.h>
#include "dtm.h"

/* -------- Public Methods -------------------------------- */

CPlayer *CdtmLoader::factory(Copl *newopl)
{
  CdtmLoader *p = new CdtmLoader(newopl);
  return p;
}

bool CdtmLoader::load(istream &f, const char *filename)
{
#ifdef _DEBUG
	DebugBreak();
#endif
	const unsigned char conv_inst[11] = { 2,1,10,9,4,3,6,5,0,8,7 };
	const unsigned short conv_note[12] = { 0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287, 0x2AE };

	int i,j,k,t=0;

	// signature exists ?
	f.read((char *)&header,sizeof(dtm_header));
	if (strncmp(header.id,"DeFy DTM ",9))
		return false;

	// good version ?
	if (header.version != 0x10)
		return false;

	header.numinst++;

	// load description
	memset(desc,0,80*16);

	char bufstr[80];
	unsigned char bufstr_length;

	for (i=0;i<16;i++)
	{
		// get line length
		bufstr_length = f.get();

		// read line
		f.read(bufstr,bufstr_length);

		// convert it
		for(j=0;j<bufstr_length;j++)
			if (!bufstr[j])
				bufstr[j] = 0x20;

		bufstr[bufstr_length] = 0;

		strcat(desc,bufstr);
		strcat(desc,"\n");
	}

	// init CmodPlayer
	realloc_instruments(header.numinst);
	realloc_order(100);
	realloc_patterns(header.numpat,64,9);

	init_notetable(conv_note);
	init_trackord();

	// load instruments
	for (i=0;i<header.numinst;i++)
	{
		f.read(instruments[i].name, f.get());
		f.read(instruments[i].data,12);

		for(j=0;j<11;j++)
			inst[i].data[conv_inst[j]] = instruments[i].data[j];
	}

	// load order
	f.read(order,100);

	nop = header.numpat;

	unsigned char *pattern = new unsigned char [0x480];

	// load tracks
	for (i=0;i<nop;i++)
	{
		unsigned short packed_length = f.get() + (f.get() << 8);

		unsigned char *packed_pattern = new unsigned char [packed_length];

		f.read((char *)packed_pattern,packed_length);

		unpack_pattern(packed_pattern,packed_length,pattern);

		delete packed_pattern;

		for (j=0;j<9;j++)
		{
			for (k=0;k<64;k++)
			{
				dtm_event *event = &pattern[(k*9 + j)*2];

				
			}

			t++;
		}
	}

	delete pattern;

	// default instruments
	for (i=0;i<9;i++)
	{
		if (!tracks[i][0].inst)
			tracks[i][0].inst = i + 1;
	}

	// order length
	for (i=0;i<100;i++)
	{
		if (order[i] >= 0x80)
		{
			length = i;

			if (order[i] == 0xFF)
				restartpos = 0;
			else
				restartpos = order[i] - 0x80;

			break;
		}
	}

	// initial speed
	initspeed = 2;

	rewind(0);

	return true;
}

float CdtmLoader::getrefresh()
{
	return 18.2f;
}

std::string CdtmLoader::gettype()
{
	return std::string("DeFy Adlib Tracker");
}

std::string CdtmLoader::gettitle()
{
	return std::string(header.title);
}

std::string CdtmLoader::getauthor()
{
	return std::string(header.author);
}

std::string CdtmLoader::getdesc()
{
	return std::string(desc);
}

std::string CdtmLoader::getinstrument(unsigned int n)
{
	return std::string(instruments[n].name);
}

unsigned int CdtmLoader::getinstruments()
{
	return header.numinst;
}

/* -------- Private Methods ------------------------------- */

long CdtmLoader::unpack_pattern(unsigned char *ibuf, long ilen, unsigned char *obuf)
{
	unsigned char *input = ibuf;
	unsigned char *output = obuf;

	unsigned char repeat_byte, repeat_counter;

	// RLE
	while ((input - ibuf) < ilen)
	{
		repeat_byte = *input++;

		if ((repeat_byte & 0xF0) == 0xD0)
		{
			repeat_counter = repeat_byte & 15;
			repeat_byte = *input++;
		}
		else
			repeat_counter = 1;

		for (int i=0;i<repeat_counter;i++)
			*output++ = repeat_byte;
	}

	return (output - obuf);
}

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
 * hsp.cpp - HSP Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include <string.h>

#include "hsp.h"

CPlayer *ChspLoader::factory(Copl *newopl)
{
  ChspLoader *p = new ChspLoader(newopl);
  return p;
}

bool ChspLoader::load(istream &f, const char *filename)
{
	unsigned short i,j,orgsize;
	unsigned long filesize;
	unsigned char *cmp,*org;

	// file validation section
	if(strlen(filename) < 4 || stricmp(filename+strlen(filename)-4,".hsp"))
	  return false;

	f.seekg(0,ios::end); filesize = f.tellg(); f.seekg(0);
	f.read((char *)&orgsize,2);
	if(orgsize > 59187)
		return false;

	// load section
	cmp = new unsigned char[filesize];
	f.read((char *)cmp,orgsize);

	org = new unsigned char[orgsize];
	for(i=0,j=0;i<filesize;j+=cmp[i],i+=2)	// decompress
		memset(org+j,cmp[i+1],cmp[i]);
	delete [] cmp;

	memcpy(instr,org,128*12);		// instruments
	for (i=0;i<128;i++) {				// correct instruments
		instr[i][2] ^= (instr[i][2] & 0x40) << 1;
		instr[i][3] ^= (instr[i][3] & 0x40) << 1;
		instr[i][11] >>= 4;			// slide
	}
	memcpy(song,org+128*12,51);	// tracklist
	memcpy(patterns,org+128*12+51,orgsize-128*12-51);	// patterns
	delete [] org;

	rewind(0);						// rewind HSC module
	return true;
}

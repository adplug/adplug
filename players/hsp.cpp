/*
 * hsp.cpp - HSP Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include <string.h>

#include "hsp.h"

bool ChspLoader::load(istream &f)
{
	unsigned short i,j,orgsize;
	unsigned long filesize;
	unsigned char *cmp,*org;

	// file validation section
	f.seekg(0,ios::end); filesize = f.tellg(); f.seekg(0);
	f.read((char *)&orgsize,2);
	if(orgsize > 59187)
		return false;

	// load section
	cmp = new unsigned char[filesize];
	f.read(cmp,orgsize);

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

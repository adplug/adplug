/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2002 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * lds.cpp - LOUDNESS Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include <string.h>

#include "lds.h"

/*** public methods *************************************/

CPlayer *CldsLoader::factory(Copl *newopl)
{
  CldsLoader *p = new CldsLoader(newopl);
  return p;
}

bool CldsLoader::load(istream &f, const char *filename)
{
	int				i,j,k,l,dnum[9][2];
	unsigned char	ldsinst[46],*ldspat,*ldsm;
	unsigned short	mlen;									// length of music data
	const char		ixlt[11] = {1,4,2,5,9,6,10,7,3,8,0};	// instrument translation map

	// file validation section (actually just an extension check)
	if(strlen(filename) < 4 || stricmp(filename+strlen(filename)-4,".lds"))
		return false;

	f.ignore(15);			// ignore header
	insts = f.get();		// get number of instruments
	for(i=0;i<insts;i++) {	// load instruments
		f.read((char *)ldsinst,46);
		for(j=0;j<11;j++)	// convert instrument
			inst[i].data[j] = ldsinst[ixlt[j]+1];
	}
	f.read((char *)&nop,2);
	ldspat = new unsigned char [nop*9*3];
	f.read((char *)ldspat,nop*9*3);
	f.read((char *)&mlen,2);
	ldsm = new unsigned char [mlen];
	f.read((char *)ldsm,mlen);

	for(i=0;i<nop;i++)
		order[i] = i;
	length = nop; restartpos = 0;
	initspeed = 6; bpm = 18; flags = Standard;
	init_trackord();		// patterns
	for(i=0;i<9;i++) {
		dnum[i][0] = -1;
		dnum[i][1] = 1;
	}
	for(i=0;i<nop;i++)
		for(j=0;j<24;j++)
			for(k=0;k<9;k++) {
				l = ((ldspat[i*9*3+k*3+1]+j*4) << 8) + (ldspat[i*9*3+k*3]+j*4);
				if(dnum[k][0] < 24) {
					dnum[k][1] = 1;
					if(ldsm[l+2])
						dnum[k][0] += dnum[k][1];
					if(dnum[k][0] >= 0 && dnum[k][0] < 24) {
						tracks[i*9+k][dnum[j][0]].note = ldsm[l+1];
						tracks[i*9+k][dnum[j][0]].inst = ldsm[l];
						tracks[i*9+k][dnum[j][0]].param1 = ldsm[l+3];
						tracks[i*9+k][dnum[j][0]].command = 17;
					}
				}
			}

	delete [] ldspat;
	delete [] ldsm;
	return true;
}

/*** private methods *************************************/

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
 * a2m.h - A2M Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include "protrack.h"

/*
#define MAXFREQ			2000
#define MINCOPY			3
#define MAXCOPY			255
#define COPYRANGES		6
#define CODESPERRANGE	(MAXCOPY - MINCOPY + 1)

#define TERMINATE		256
#define FIRSTCODE		257
#define MAXCHAR			(FIRSTCODE + COPYRANGES * CODESPERRANGE - 1)
#define SUCCMAX			(MAXCHAR + 1)
#define TWICEMAX		(2 * MAXCHAR + 1)
#define ROOT			1
#define MAXBUF			(42 * 1024)

#define MAXDISTANCE		21389
#define MAXSIZE			21389 + MAXCOPY
*/

class Ca2mLoader: public CmodPlayer
{
public:
  static CPlayer *factory(Copl *newopl);

	Ca2mLoader(Copl *newopl)
		: CmodPlayer(newopl)
	{ };

	bool load(const std::string &filename, const CFileProvider &fp);
	float getrefresh();

	std::string gettype()
	{ return std::string("AdLib Tracker 2"); };
	std::string gettitle()
	{ if(*songname) return std::string(songname,1,*songname); else return std::string(); };
	std::string getauthor()
	{ if(*author) return std::string(author,1,*author); else return std::string(); };
	unsigned int getinstruments()
	{ return 250; };
	std::string getinstrument(unsigned int n)
	{ return std::string(instname[n],1,*instname[n]); };

private:
	static const unsigned int MAXFREQ = 2000, MINCOPY = 3, MAXCOPY = 255,
	  COPYRANGES = 6, CODESPERRANGE = MAXCOPY - MINCOPY + 1, TERMINATE = 256,
	  FIRSTCODE = 257, MAXCHAR = FIRSTCODE + COPYRANGES * CODESPERRANGE - 1,
	  SUCCMAX = MAXCHAR + 1, TWICEMAX = 2 * MAXCHAR + 1, ROOT = 1,
	  MAXBUF = 42 * 1024, MAXDISTANCE = 21389, MAXSIZE = 21389 + MAXCOPY;

	static const unsigned short bitvalue[14];
	static const signed short copybits[COPYRANGES], copymin[COPYRANGES];

	void inittree();
	void updatefreq(unsigned short a,unsigned short b);
	void updatemodel(unsigned short code);
	unsigned short inputcode(unsigned short bits);
	unsigned short uncompress();
	void decode();
	unsigned short sixdepak(unsigned short *source,unsigned char *dest,unsigned short size);

	char songname[43],author[43],instname[250][33];

	unsigned short ibitcount,ibitbuffer,ibufcount,obufcount,input_size,output_size,leftc[MAXCHAR+1],
		rghtc[MAXCHAR+1],dad[TWICEMAX+1],freq[TWICEMAX+1],*wdbuf;
	unsigned char *obuf,*buf;
};

/*
 * a2m.h - A2M Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include "protrack.h"

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

class Ca2mLoader: public CmodPlayer
{
public:
	Ca2mLoader(Copl *newopl)
		: CmodPlayer(newopl)
	{ };

	bool load(istream &f);
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

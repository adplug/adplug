/*
 * ksm.h - KSM Player by Simon Peter (dn.tlp@gmx.net)
 */

#include "player.h"

class CksmPlayer: public CPlayer
{
public:
	CksmPlayer(Copl *newopl)
		: CPlayer(newopl), note(0)
	{ };
	~CksmPlayer()
	{ if(note) delete [] note; };

	bool load(istream &f);
	bool update();
	void rewind(unsigned int subsong);
	float getrefresh()
	{ return 240.0f; };

	std::string gettype()
	{ return std::string("Ken Silverman's Music Format"); };
	unsigned int getinstruments()
	{ return 16; };
	std::string getinstrument(unsigned int n);

	void loadinsts(istream &f);

private:
	unsigned long count,countstop,chanage[18],*note;
	unsigned short numnotes;
	unsigned int nownote,numchans,drumstat;
	unsigned char trinst[16],trquant[16],trchan[16],trvol[16],inst[256][11],databuf[2048],chanfreq[18],chantrack[18];
	char instname[256][20];

	bool songend;

	void setinst(int chan,unsigned char v0,unsigned char v1,unsigned char v2,unsigned char v3,
				 unsigned char v4,unsigned char v5,unsigned char v6,unsigned char v7,
				 unsigned char v8,unsigned char v9,unsigned char v10);
};

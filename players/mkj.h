/*
 * mkj.h - MKJamz Player, by Simon Peter (dn.tlp@gmx.net)
 */

#include "player.h"

class CmkjPlayer: public CPlayer
{
public:
	CmkjPlayer(Copl *newopl)
		: CPlayer(newopl), songbuf(0)
	{ };
	~CmkjPlayer()
	{ if(songbuf) delete [] songbuf; };

	bool load(istream &f);
	bool update();
	void rewind(unsigned int subsong);
	float getrefresh();

	std::string gettype()
	{ return std::string("MKJamz Audio File"); };

private:
	short maxchannel,maxnotes,*songbuf;
	bool songend;

	struct {
		short defined,songptr,octave,waveform,pstat,speed;
		bool isplaying,flag,isdone;
	} channel[9];
};

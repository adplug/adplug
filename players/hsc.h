/*
 * hsc.h - HSC Player by Simon Peter (dn.tlp@gmx.net)
 */

#ifndef H_HSCPLAYER
#define H_HSCPLAYER

#include "player.h"

class ChscPlayer: public CPlayer
{
public:
	ChscPlayer(Copl *newopl)
		: CPlayer(newopl), mtkmode(0)
	{ };

	bool load(istream &f);
	bool update();
	void rewind(unsigned int subsong);
	float getrefresh()
	{ return 18.2f; };	// refresh rate is fixed at 18.2Hz

	std::string gettype()
	{ return std::string("HSC Adlib Composer / HSC-Tracker"); };
	unsigned int getpatterns();
	unsigned int getpattern()
	{ return song[songpos]; };
	unsigned int getorders();
	unsigned int getorder()
	{ return songpos; };
	unsigned int getrow()
	{ return pattpos; };
	unsigned int getspeed()
	{ return speed; };
	unsigned int getinstruments();

protected:
	struct hscnote {
		unsigned char note, effect;
	};			// note type in HSC pattern

	struct hscchan {
	    unsigned char inst;			// current instrument
	    signed char slide;			// used for manual slide-effects
	    unsigned short freq;		// actual replaying frequency
	};			// HSC channel data

	hscchan channel[9];				// player channel-info
	unsigned char instr[128][12];	// instrument data
	unsigned char song[0x80];		// song-arrangement (MPU-401 Trakker enhanced)
	hscnote patterns[50][64*9];		// pattern data
	unsigned char pattpos,songpos,	// various bytes & flags
		pattbreak,songend,mode6,bd,fadein;
	unsigned int speed,del;
	unsigned char adl_freq[9];		// adlib frequency registers
	int mtkmode;					// flag: MPU-401 Trakker mode on/off

private:
	void setfreq(unsigned char chan, unsigned short freq);
	void setvolume(unsigned char chan, int volc, int volm);
	void setinstr(unsigned char chan, unsigned char insnr);
};

#endif

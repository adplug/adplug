/*
 * protrack.h - Generic Protracker Player by Simon Peter (dn.tlp@gmx.net)
 */

#ifndef H_PROTRACK
#define H_PROTRACK

#include "player.h"

#define MOD_FLAGS_DECIMAL	1

class CmodPlayer: public CPlayer
{
public:
	CmodPlayer(Copl *newopl)
		: CPlayer(newopl), flags(0), initspeed(6)
	{ memset(inst,0,sizeof(inst)); };

	bool update();
	void rewind(unsigned int subsong);
	float getrefresh();

	unsigned int getpatterns()
	{ return nop; };
	unsigned int getpattern()
	{ return order[ord]; };
	unsigned int getorders()
	{ return length; };
	unsigned int getorder()
	{ return ord; };
	unsigned int getrow()
	{ return rw; };
	unsigned int getspeed()
	{ return speed; };

protected:
	struct {
		unsigned short freq,nextfreq;
		unsigned char oct,vol1,vol2,inst,fx,info1,info2,key,nextoct,note,portainfo,vibinfo1,vibinfo2,arppos,arpspdcnt;
		signed char trigger;
	} channel[9];

	struct {
		unsigned char data[11],arpstart,arpspeed,arppos,arpspdcnt,misc;
		signed char slide;
	} inst[250];

	struct {
		unsigned char note,command,inst,param2,param1;
	} tracks[576][64];

	unsigned char order[128],arplist[256],arpcmd[256],rw,ord,speed,del,songend,regbd,length,restartpos,initspeed;
	unsigned short tempo,activechan,trackord[64][9],nop,bpm,flags;

private:
	void setvolume(unsigned char chan);
	void setfreq(unsigned char chan);
	void playnote(unsigned char chan);
	void setnote(unsigned char chan, int note);
	void slide_down(unsigned char chan, int amount);
	void slide_up(unsigned char chan, int amount);
	void tone_portamento(unsigned char chan, unsigned char info);
	void vibrato(unsigned char chan, unsigned char speed, unsigned char depth);
	void vol_up(unsigned char chan, int amount);
	void vol_down(unsigned char chan, int amount);
	void vol_up_alt(unsigned char chan, int amount);
	void vol_down_alt(unsigned char chan, int amount);
};

#endif

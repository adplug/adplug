/*
 * s3m.h - AdLib S3M Player by Simon Peter (dn.tlp@gmx.net)
 */

#include "player.h"

class Cs3mPlayer: public CPlayer
{
public:
	Cs3mPlayer(Copl *newopl);

	bool load(istream &f);
	bool update();
	void rewind(unsigned int subsong);
	float getrefresh();

	std::string gettype();
	std::string gettitle()
	{ return std::string(header.name); };

	unsigned int getpatterns()
	{ return header.patnum; };
	unsigned int getpattern()
	{ return orders[ord]; };
	unsigned int getorders()
	{ return (header.ordnum-1); };
	unsigned int getorder()
	{ return ord; };
	unsigned int getrow()
	{ return crow; };
	unsigned int getspeed()
	{ return speed; };
	unsigned int getinstruments()
	{ return header.insnum; };
	std::string getinstrument(unsigned int n)
	{ return std::string(inst[n].name); };

protected:
	struct s3mheader {
		char name[28];				// song name
		unsigned char kennung,typ,dummy[2];
		unsigned short ordnum,insnum,patnum,flags,cwtv,ffi;
		char scrm[4];
		unsigned char gv,is,it,mv,uc,dp,dummy2[8];
		unsigned short special;
		unsigned char chanset[32];
	};

	struct {
		unsigned char type;
		char filename[15];
		unsigned char d00,d01,d02,d03,d04,d05,d06,d07,d08,d09,d0a,d0b,volume,dsk,dummy[2];
		unsigned long c2spd;
		char dummy2[12],name[28],scri[4];
	} inst[99];

	struct {
		unsigned char note,oct,instrument,volume,command,info;
	} pattern[99][64][32];

	struct {
		unsigned short freq,nextfreq;
		unsigned char oct,vol,inst,fx,info,dualinfo,key,nextoct,trigger,note;
	} channel[9];

	s3mheader header;
	unsigned char orders[256];
	unsigned char crow,ord,speed,tempo,del,songend,loopstart,loopcnt;

private:
	void setvolume(unsigned char chan);
	void setfreq(unsigned char chan);
	void playnote(unsigned char chan);
	void slide_down(unsigned char chan, unsigned char amount);
	void slide_up(unsigned char chan, unsigned char amount);
	void vibrato(unsigned char chan, unsigned char info);
	void tone_portamento(unsigned char chan, unsigned char info);
};

/*
 * d00.h - D00 Player by Simon Peter (dn.tlp@gmx.net)
 */

#include "player.h"

class Cd00Player: public CPlayer
{
public:
	Cd00Player(Copl *newopl)
		: CPlayer(newopl), filedata(0)
	{ };
	~Cd00Player()
	{ if(filedata) delete [] filedata; };

	bool load(istream &f);
	bool update();
	void rewind(unsigned int subsong);
	float getrefresh();

	std::string gettype();
	std::string gettitle()
	{ if(version > 1) return std::string(header->songname); else return std::string(); };
	std::string getauthor()
	{ if(version > 1) return std::string(header->author); else return std::string(); };
	std::string getdesc()
	{ if(*datainfo) return std::string(datainfo); else return std::string(); };
	unsigned int getsubsongs();

protected:

// MS C and WATCOM C both align their structs, i don't know about other compilers
#if defined(_MSC_VER) || defined(__WATCOMC__)
#pragma pack(1)		// following structs MUST be byte-aligned!
#endif

	struct d00header {
		char id[6];
		unsigned char type,version,speed,subsongs,soundcard;
		char songname[32],author[32],dummy[32];
		unsigned short tpoin,seqptr,instptr,infoptr,spfxptr,endmark;
	};

	struct d00header1 {
		unsigned char version,speed,subsongs;
		unsigned short tpoin,seqptr,instptr,infoptr,lpulptr,endmark;
	};

	struct {
		unsigned short	*order,ordpos,pattpos,del,speed,rhcnt,key,freq,inst,spfx,ispfx,irhcnt;
		signed short	transpose,slide,slideval,vibspeed;
		unsigned char	seqend,vol,vibdepth,fxdel,modvol,cvol,levpuls,frameskip,nextnote,note,ilevpuls,trigger;
	} channel[9];

	struct Sinsts {
		unsigned char data[11],tunelev,timer,sr,dummy[2];
	} *inst;

	struct Sspfx {
		unsigned short instnr;
		signed char halfnote;
		unsigned char modlev;
		signed char modlevadd;
		unsigned char duration;
		unsigned short ptr;
	} *spfx;

	struct Slevpuls {
		unsigned char level;
		signed char voladd;
		unsigned char duration,ptr;
	} *levpuls;

	unsigned char songend,version;
	char *datainfo;
	unsigned short *seqptr;
	d00header *header;
	d00header1 *header1;
	void *filedata;

#if defined(_MSC_VER) || defined(__WATCOMC__)
#pragma pack()		// revert to old alignment
#endif

private:
	void setvolume(unsigned char chan);
	void setfreq(unsigned char chan);
	void setinst(unsigned char chan);
	void playnote(unsigned char chan);
	void vibrato(unsigned char chan);
};

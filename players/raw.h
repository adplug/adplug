/*
 * raw.h - RAW Player by Simon Peter (dn.tlp@gmx.net)
 */

#include "player.h"

class CrawPlayer: public CPlayer
{
public:
	CrawPlayer(Copl *newopl)
		: CPlayer(newopl), data(0)
	{ };
	~CrawPlayer()
	{ if(data) delete [] data; };

	bool load(istream &f);
	bool update();
	void rewind(unsigned int subsong);
	float getrefresh();

	std::string gettype()
	{ return std::string("RdosPlay RAW"); };

protected:
	struct Tdata {
		unsigned char param,command;
	} *data;

	unsigned long pos,length;
	unsigned short clock,speed;
	unsigned char del,opl3;
	bool songend;
};

/*
 * sng.h - SNG Player by Simon Peter (dn.tlp@gmx.net)
 */

#include "player.h"

class CsngPlayer: public CPlayer
{
public:
	CsngPlayer(Copl *newopl)
		: CPlayer(newopl)
	{ };
	~CsngPlayer()
	{ if(data) delete [] data; };

	bool load(istream &f);
	bool update();
	void rewind(unsigned int subsong);
	float getrefresh()
	{ return 70.0f; };

	std::string gettype()
	{ return std::string("SNG File Format"); };

protected:
	struct {
		char id[4];
		unsigned short length,start,loop;
		unsigned char delay;
		bool compressed;
	} header;

	struct Sdata {
		unsigned char val,reg;
	} *data;

	unsigned char del;
	unsigned short pos;
	bool songend;
};

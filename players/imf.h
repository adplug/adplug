/*
 * imf.h - IMF Player by Simon Peter (dn.tlp@gmx.net)
 */

#include "player.h"

class CimfPlayer: public CPlayer
{
public:
	CimfPlayer(Copl *newopl)
		: CPlayer(newopl), data(0)
	{ };
	~CimfPlayer()
	{ if(data) delete [] data; };

	bool load(istream &f);
	bool update();
	void rewind(unsigned int subsong);
	float getrefresh()
	{ return timer; };

	std::string gettype()
	{ return std::string("IMF File Format"); };

protected:
	unsigned long pos,size;
	unsigned short del;
	bool songend;
	float rate,timer;

	struct Sdata {
		unsigned char reg,val;
		unsigned short time;
	} *data;

private:
	unsigned long crc32(unsigned char *buf, unsigned long size);
	float getrate(unsigned long crc, unsigned long size);
};

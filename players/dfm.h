/*
 * dfm.h - Digital-FM Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include "protrack.h"

class CdfmLoader: public CmodPlayer
{
public:
	CdfmLoader(Copl *newopl)
		: CmodPlayer(newopl)
	{ };

	bool load(istream &f);
	float getrefresh();

	std::string gettype();
	unsigned int getinstruments()
	{ return 32; };
	std::string getinstrument(unsigned int n)
	{ return std::string(instname[n],1,*instname[n]); };
	std::string getdesc()
	{ return std::string(songinfo,1,*songinfo); };

private:
	struct {
		char id[4];
		unsigned char hiver,lover;
	} header;

	char songinfo[33];
	char instname[32][12];
};

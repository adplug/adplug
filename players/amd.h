/*
 * amd.h - AMD Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include "protrack.h"

class CamdLoader: public CmodPlayer
{
public:
	CamdLoader(Copl *newopl)
		: CmodPlayer(newopl)
	{ };

	bool load(istream &f);
	float getrefresh();

	std::string gettype()
	{ return std::string("AMUSIC Adlib Tracker"); };
	std::string gettitle()
	{ return std::string(songname,0,24); };
	std::string getauthor()
	{ return std::string(author,0,24); };
	unsigned int getinstruments()
	{ return 26; };
	std::string getinstrument(unsigned int n)
	{ return std::string(instname[n],0,23); };

private:
	char songname[24],author[24],instname[26][23];
	unsigned char version;
};

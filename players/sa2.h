/*
 * sa2.h - SAdT2 Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include "protrack.h"

class Csa2Loader: public CmodPlayer
{
public:
	Csa2Loader(Copl *newopl)
		: CmodPlayer(newopl)
	{ };

	bool load(istream &f);

	std::string gettype();
	std::string gettitle();
	unsigned int getinstruments()
	{ return 29; };
	std::string getinstrument(unsigned int n)
	{ return std::string(instname[n],1,16); };

private:
	struct sa2header {
		char sadt[4];
		unsigned char version;
	} header;

	char instname[29][17];
};

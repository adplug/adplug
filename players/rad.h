/*
 * rad.h - RAD Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include "protrack.h"

class CradLoader: public CmodPlayer
{
public:
	CradLoader(Copl *newopl)
		: CmodPlayer(newopl)
	{ *desc = '\0'; };

	bool load(istream &f);
	float getrefresh();

	std::string gettype()
	{ return std::string("Reality ADlib Tracker"); };
	std::string getdesc()
	{ return std::string(desc); };

private:
	unsigned char version,radflags;
	char desc[80*22];
};

/*
 * mtk.h - MPU-401 Trakker Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include "hsc.h"

class CmtkLoader: public ChscPlayer
{
public:
	CmtkLoader(Copl *newopl)
		: ChscPlayer(newopl)
	{
		mtkmode = 1;
	};

	bool load(istream &f);

	std::string gettype()
	{ return std::string("MPU-401 Trakker"); };
	std::string gettitle()
	{ return std::string(title); };
	std::string getauthor()
	{ return std::string(composer); };
	unsigned int getinstruments()
	{ return 128; };
	std::string getinstrument(unsigned int n)
	{ return std::string(instname[n]); };

private:
	char title[34],composer[34],instname[0x80][34];
};

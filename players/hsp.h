/*
 * hsp.h: HSC Packed Loader by Simon Peter (dn.tlp@gmx.net)
 */

#include "hsc.h"

class ChspLoader: public ChscPlayer
{
public:
	ChspLoader(Copl *newopl)
		: ChscPlayer(newopl)
	{};

	bool load(istream &f);
};

/*
 * adplug.h - CAdPlug main class declaration, by Simon Peter (dn.tlp@gmx.net)
 */

#include <iostream.h>

#include "./players/player.h"
#include "opl.h"

#define DFLSUBSONG		0					// default subsong to start with

class CAdPlug
{
public:
	CPlayer *factory(char *fn, Copl *opl);
	CPlayer *factory(istream &f, Copl *opl);

	unsigned long songlength(CPlayer *p, unsigned int subsong = 0xffff);
	void seek(CPlayer *p, unsigned long ms);

private:
	char	*upstr(char *str);							// converts a string to all uppercase letters
	CPlayer	*load_sci(istream &f, char *fn, Copl *opl);	// special loader for Sierra SCI file format
	CPlayer	*load_ksm(istream &f, char *fn, Copl *opl);	// special loader for Ken Silverman's music format
};

/*
 * player.h - replayer base class, by Simon Peter (dn.tlp@gmx.net)
 */

#ifndef H_PLAYER
#define H_PLAYER

#include <iostream.h>
#include <string>
#include "../opl.h"

// standard adlib note table
static const unsigned short note_table[12] = {363,385,408,432,458,485,514,544,577,611,647,686};
// the 9 operators as expected by the OPL2
static const unsigned char op_table[9] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

class CPlayer
{
public:
	CPlayer(Copl *newopl)											// newopl = OPL chip to use
		: opl(newopl)
	{ };
	virtual ~CPlayer()
	{ };

	virtual bool load(istream &f) = 0;								// loads file
	virtual bool update() = 0;										// executes replay code for 1 tick
	virtual void rewind(unsigned int subsong = 0xffff) = 0;			// rewinds to specified subsong
	virtual float getrefresh() = 0;									// returns needed timer refresh rate

	virtual std::string gettype() = 0;								// returns file type
	virtual std::string gettitle()									// returns song title
	{ return std::string(); };
	virtual std::string getauthor()									// returns song author name
	{ return std::string(); };
	virtual std::string getdesc()									// returns song description
	{ return std::string(); };
	virtual unsigned int getpatterns()								// returns number of patterns
	{ return 0; };
	virtual unsigned int getpattern()								// returns currently playing pattern
	{ return 0; };
	virtual unsigned int getorders()								// returns size of orderlist
	{ return 0; };
	virtual unsigned int getorder()									// returns currently playing song position
	{ return 0; };
	virtual unsigned int getrow()									// returns currently playing row
	{ return 0; };
	virtual unsigned int getspeed()									// returns current song speed
	{ return 0; };
	virtual unsigned int getsubsongs()								// returns number of subsongs
	{ return 1; };
	virtual unsigned int getinstruments()							// returns number of instruments
	{ return 0; };
	virtual std::string getinstrument(unsigned int n)				// returns n-th instrument name
	{ return std::string(); };

protected:
	Copl *opl;		// our OPL chip
};

#endif

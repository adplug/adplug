/*
 * opl.h - OPL base class declaration, by Simon Peter (dn.tlp@gmx.net)
 */

#ifndef H_OPL
#define H_OPL

class Copl
{
public:
	virtual void write(int reg, int val) = 0;	// combined register select + data write
	virtual void init(void) = 0;				// reinitialize OPL chip
};

#endif

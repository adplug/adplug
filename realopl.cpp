/*
 * realopl.cpp - Real hardware OPL, by Simon Peter (dn.tlp@gmx.net)
 */

#include <conio.h>
#include "realopl.h"

#ifdef _MSC_VER
	#define INP		_inp
	#define OUTP	_outp
#elif defined(__WATCOMC__)
	#define INP		inp
	#define OUTP	outp
#endif

#define SHORTDELAY		8					// short delay in microseconds after OPL hardware output
#define LONGDELAY		35					// long delay in microseconds after OPL hardware output

// the 9 operators as expected by the OPL2
static const unsigned char op_table[9] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

CRealopl::CRealopl(unsigned short initport): adlport(initport), hardvol(0), bequiet(false), nowrite(false)
{
	for(int i=0;i<22;i++) {
		hardvols[i][0] = 0;
		hardvols[i][1] = 0;
	}
}

bool CRealopl::detect()
{
	unsigned char stat1,stat2,i;

	hardwrite(4,0x60); hardwrite(4,0x80);
	stat1 = INP(adlport);
	hardwrite(2,0xff); hardwrite(4,0x21);
	for(i=0;i<80;i++)			// wait for adlib
		INP(adlport);
	stat2 = INP(adlport);
	hardwrite(4,0x60); hardwrite(4,0x80);

	if(((stat1 & 0xe0) == 0) && ((stat2 & 0xe0) == 0xc0))
		return true;
	else
		return false;
}

void CRealopl::setvolume(int volume)
{
	int i;

	hardvol = volume;
	for(i=0;i<9;i++) {
		hardwrite(0x43+op_table[i],((hardvols[op_table[i]+3][0] & 63) + volume) > 63 ? 63 : hardvols[op_table[i]+3][0] + volume);
		if(hardvols[i][1] & 1)	// modulator too?
			hardwrite(0x40+op_table[i],((hardvols[op_table[i]][0] & 63) + volume) > 63 ? 63 : hardvols[op_table[i]][0] + volume);
	}
}

void CRealopl::setquiet(bool quiet)
{
	bequiet = quiet;

	if(quiet) {
		oldvol = hardvol;
		setvolume(63);
	} else
		setvolume(oldvol);
}

void CRealopl::hardwrite(int reg, int val)
{
	int i;

	OUTP(adlport,reg);							// set register
	for(i=0;i<SHORTDELAY;i++)					// wait for adlib
		INP(adlport);
	OUTP(adlport+1,val);						// set value
	for(i=0;i<LONGDELAY;i++)					// wait for adlib
		INP(adlport);
}

void CRealopl::write(int reg, int val)
{
	int i;

	if(nowrite)
		return;

	if(bequiet && (reg >= 0xb0 && reg <= 0xb8))	// filter all key-on commands
		val &= ~32;
	if(reg >= 0x40 && reg <= 0x55)				// cache volumes
		hardvols[reg-0x40][0] = val;
	if(reg >= 0xc0 && reg <= 0xc8)
		hardvols[reg-0xc0][1] = val;
	if(hardvol)									// reduce volume
		for(i=0;i<9;i++) {
			if(reg == 0x43 + op_table[i])
				val = ((val & 63) + hardvol) > 63 ? 63 : val + hardvol;
			else
				if((reg == 0x40 + op_table[i]) && (hardvols[i][1] & 1))
					val = ((val & 63) + hardvol) > 63 ? 63 : val + hardvol;
		}

	hardwrite(reg,val);
}

void CRealopl::init()
{
	int i;

	for (i=0;i<9;i++) {				// stop instruments
		hardwrite(0xb0 + i,0);				// key off
		hardwrite(0x80 + op_table[i],0xff);	// fastest release
	}
	hardwrite(0xbd,0);	// clear misc. register
}

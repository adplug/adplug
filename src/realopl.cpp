/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2004 Simon Peter, <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * realopl.cpp - Real hardware OPL, by Simon Peter <dn.tlp@gmx.net>
 */

#ifdef _MSC_VER			// Microsoft Visual C++
#	include <conio.h>
#	define INP	_inp
#	define OUTP	_outp
#elif defined(__WATCOMC__)	// Watcom C/C++ and OpenWatcom
#	include <conio.h>
#	define INP	inp
#	define OUTP	outp
#elif defined(WIN32) && defined(__MSVCRT__) && defined(__MINGW32__)
/*
int __cdecl _inp(unsigned short);
int __cdecl _outp(unsigned short, int);
#	define INP	_inp
#	define OUTP	_outp
*/
#	define INP		inb
#	define OUTP(reg, val)	outb(val, reg)
#else				// no support on other platforms
#	define INP(reg)		0
#	define OUTP(reg, val)
#endif

#include "realopl.h"

#define SHORTDELAY		6	// short delay in I/O port-reads after OPL hardware output
#define LONGDELAY		35	// long delay in I/O port-reads after OPL hardware output

const unsigned char CRealopl::op_table[9] =
  {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

#if defined(WIN32) && defined(__MINGW32__)
static __inline unsigned char
inb (unsigned short int port)
{
  unsigned char _v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static __inline void
outb (unsigned char value, unsigned short int port)
{
  __asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}
#endif

CRealopl::CRealopl(unsigned short initport)
  : adlport(initport), hardvol(0), bequiet(false), nowrite(false)
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

  OUTP(adlport,reg);		// set register
  for(i=0;i<SHORTDELAY;i++)	// wait for adlib
    INP(adlport);
  OUTP(adlport+1,val);		// set value
  for(i=0;i<LONGDELAY;i++)	// wait for adlib
    INP(adlport);
}

void CRealopl::write(int reg, int val)
{
  int i;

  if(nowrite)
    return;

  if(bequiet && (reg >= 0xb0 && reg <= 0xb8))	// filter all key-on commands
    val &= ~32;
  if(reg >= 0x40 && reg <= 0x55)		// cache volumes
    hardvols[reg-0x40][0] = val;
  if(reg >= 0xc0 && reg <= 0xc8)
    hardvols[reg-0xc0][1] = val;
  if(hardvol)					// reduce volume
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

  for (i=0;i<9;i++) {			// stop instruments
    hardwrite(0xb0 + i,0);		// key off
    hardwrite(0x80 + op_table[i],0xff);	// fastest release
  }
  hardwrite(0xbd,0);	// clear misc. register
}

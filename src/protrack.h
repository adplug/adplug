/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2002 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * protrack.h - Generic Protracker Player by Simon Peter <dn.tlp@gmx.net>
 */

#ifndef H_PROTRACK
#define H_PROTRACK

#include "player.h"

/*
 * Use of the MOD_FLAGS_* defines is deprecated!
 * Use the 'enum Flags' below instead.
 */
#define MOD_FLAGS_STANDARD	0
#define MOD_FLAGS_DECIMAL	1
#define MOD_FLAGS_FAUST		2

class CmodPlayer: public CPlayer
{
public:
	CmodPlayer(Copl *newopl);

	bool update();
	void rewind(unsigned int subsong);
	float getrefresh();

	unsigned int getpatterns()
	{ return nop; };
	unsigned int getpattern()
	{ return order[ord]; };
	unsigned int getorders()
	{ return length; };
	unsigned int getorder()
	{ return ord; };
	unsigned int getrow()
	{ return rw; };
	unsigned int getspeed()
	{ return speed; };

protected:
	enum Flags {Standard = 0, Decimal, Faust};

	struct {
		unsigned short freq,nextfreq;
		unsigned char oct,vol1,vol2,inst,fx,info1,info2,key,nextoct,note,portainfo,vibinfo1,vibinfo2,arppos,arpspdcnt;
		signed char trigger;
	} channel[9];

	struct {
		unsigned char data[11],arpstart,arpspeed,arppos,arpspdcnt,misc;
		signed char slide;
	} inst[250];

	struct {
		unsigned char note,command,inst,param2,param1;
	} tracks[576][64];

	unsigned char order[128],arplist[256],arpcmd[256],rw,ord,speed,del,songend,regbd,length,restartpos,initspeed;
	unsigned short tempo,activechan,trackord[64][9],nop,bpm,flags;

	void init_trackord();

private:
	void setvolume(unsigned char chan);
	void setvolume_alt(unsigned char chan);
	void setfreq(unsigned char chan);
	void playnote(unsigned char chan);
	void setnote(unsigned char chan, int note);
	void slide_down(unsigned char chan, int amount);
	void slide_up(unsigned char chan, int amount);
	void tone_portamento(unsigned char chan, unsigned char info);
	void vibrato(unsigned char chan, unsigned char speed, unsigned char depth);
	void vol_up(unsigned char chan, int amount);
	void vol_down(unsigned char chan, int amount);
	void vol_up_alt(unsigned char chan, int amount);
	void vol_down_alt(unsigned char chan, int amount);
};

#endif

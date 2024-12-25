/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2024 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * plx.cpp - PALLADIX Player by Simon Peter <simonpeter303@gmail.com>
 */

#include <string.h>
#include <assert.h>

#include "plx.h"
#include "debug.h"

// These frequencies are written directly into the low (A0h) and high
// byte (B0h) AdLib frequency registers. They include the frequency block.
const unsigned short CplxPlayer::frequency[] = {
  343, 363, 385, 408, 432, 458, 485, 514, 544, 577, 611, 647,
  1367, 1387, 1409, 1432, 1456, 1482, 1509, 1538, 1568, 1601, 1635, 1671,
  2391, 2411, 2433, 2456, 2480, 2506, 2533, 2562, 2592, 2625, 2659, 2695,
  3415, 3435, 3457, 3480, 3504, 3530, 3557, 3586, 3616, 3649, 3683, 3719,
  4439, 4459, 4481, 4504, 4528, 4554, 4581, 4610, 4640, 4673, 4707, 4743,
  5463, 5483, 5505, 5528, 5552, 5578, 5605, 5634, 5664, 5697, 5731, 5767,
  6487, 6507, 6529, 6552, 6576, 6602, 6629, 6658, 6688, 6721, 6755, 6791,
  7511, 7531, 7553, 7576, 7600, 7626, 7653, 7682, 7712, 7745, 7779, 7815
};

const unsigned char CplxPlayer::opl2_init_regs[] = {
  0xff, 0x20, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x40, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xff, 0xff, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xff, 0xff,
  0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xff, 0xff,
  0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xff, 0xff, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xff, 0xff,
  0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x57, 0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x09, 0x0e, 0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/*** public methods *************************************/

CplxPlayer::CplxPlayer(Copl *newopl)
  : CPlayer(newopl), songdata(0), speed(1), speed_scale(1), songpos(0), song(0)
{
}

CplxPlayer::~CplxPlayer()
{
  if(song) delete song;
  if(songdata) delete [] songdata;
}

bool CplxPlayer::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f;
  char plxid[3];

  // file validation section
  f = fp.open(filename); if(!f) return false;
  f->readString(plxid, 3);
  type = f->readInt(1);
  if(strncmp(plxid, "PLX", 3) != 0 || type != 0) {
    fp.close(f);
    return false;
  }

  // file load section (header)
  speed_scale = f->readInt(1);		// Speed scale factor
  speed = f->readInt(2);		// Speed

  // Starting file offsets for all 9 channels
  for(int i = 0; i < 9; i++) {
    chan_start_offset[i] = f->readInt(2);
  }

  AdPlug_LogWrite("CplxPlayer::load(\"%s\",fp): loading PALLADIX file\n",
		  filename.c_str());

  if (!speed_scale)
  {
    AdPlug_LogWrite("Detected speed_scale==0, adjust to 1 to avoid division by zero\n");
    speed_scale = 1;
  }

  if (!speed)
  {
    AdPlug_LogWrite("Detected speed==0, adjust to 1 to avoid division by zero\n");
    speed = 1;
  }

  // Load entire song into memory
  unsigned long filesize = fp.filesize(f);
  songdata = new unsigned char [filesize];
  f->seek(0);
  f->readString((char *)songdata, filesize);
  song = new binisstream(songdata, filesize);
  assert(song != 0);

  fp.close(f);
  rewind(0);
  return true;
}

bool CplxPlayer::update()
{
  unsigned long oldpos;
  bool endsong = false;

  // For all channels
  for(int i = 0; i < 9; i++) {
    // Only if chan_offset was non-zero for this channel
    if(chan_offset[i] == 0) {
      continue;
    }

    // If we haven't reached the channel's pattern position, we skip
    if(songpos < chan_pos[i]) {
      continue;
    }

    // Read next flags
    song->seek(chan_offset[i]);
    unsigned char flags = song->readInt(1);

    if(flags != 0x80) {		// If bit 7 is set, we skip
      if(flags == 0) {			// Reset channel / song end
	chan_offset[i] = chan_start_offset[i];
	setregs(0xb0 + i, fmchip[0xb0 + i] & ~(1 << 5));	// Key off
	endsong = true;
	continue;
      }

      if(flags & (1 << 0)) {		// Set instrument
	unsigned short inst_off = song->readInt(2);
	oldpos = song->pos();

	song->seek(inst_off + 1);	// Skip 1 byte
	unsigned char feedback_alg = song->readInt(1);

	// Set modulator
	setregs(0x20 + op_table[i], song->readInt(1));
	setregs(0x40 + op_table[i], song->readInt(1));
	setregs(0x60 + op_table[i], song->readInt(1));
	setregs(0x80 + op_table[i], song->readInt(1));
	setregs(0xe0 + op_table[i], song->readInt(1));

	// Set feedback & algorithm
	setregs(0xc0 + i, feedback_alg);

	// Set carrier
	setregs(0x23 + op_table[i], song->readInt(1));
	chan_volume[i] = song->readInt(1); setregs(0x43 + op_table[i], chan_volume[i]);
	setregs(0x63 + op_table[i], song->readInt(1));
	setregs(0x83 + op_table[i], song->readInt(1));
	setregs(0xe3 + op_table[i], song->readInt(1));

	song->seek(oldpos);
      }

      if(flags & (1 << 1)) {		// Set volume
	chan_volume[i] = song->readInt(1);
	setregs(0x43 + op_table[i], chan_volume[i]);
      }

      if(flags & (1 << 2)) {		// Key off
	if(fmchip[0xb0 + i] & (1 << 5))		// If key on is currently set
	  setregs(0xb0 + i, fmchip[0xb0 + i] & ~(1 << 5));		// Mask out "key on" bit
      }

      if(flags & (7 << 3)) {		// Make any sound?
	unsigned short freq = (fmchip[0xb0 + i] << 8) | fmchip[0xa0 + i];

	if(flags & (1 << 3)) {		// Set note
	  unsigned char note = song->readInt(1);
	  assert(note % 2 == 0);	// Must be divisible by 2
	  note /= 2;
	  assert(note < 96);
	  freq = frequency[note];
	}

	if(flags & (1 << 4)) {		// Set frequency
	  freq = song->readInt(2);
	}

	if(flags & (1 << 5)) {		// Key on
	  freq |= 1 << 13;
	}

	setregs(0xa0 + i, freq & 0xff);		// Set frequency low byte
	setregs(0xb0 + i, (freq >> 8));		// Set frequency high byte
      }

      if(flags & (1 << 6)) {		// Set global tempo
	speed = song->readInt(2);
        if (!speed) {
          AdPlug_LogWrite("Detected speed==0, adjust to 1 to avoid division by zero\n");
          speed = 1;
        }
      }
    }

    // How many pattern positions to skip?
    chan_pos[i] += song->readInt(1);
    chan_offset[i] = song->pos();
  }

  songpos++;
  return !endsong;
}

void CplxPlayer::rewind(int subsong)
{
  opl->init();				// Reset OPL chip

  memset(fmchip, 0, sizeof(fmchip));

  // Init OPL2 registers
  for(int i = 0; i < 0x100; i++) {
    // if(opl2_init_regs[i] != 0xff) {
      fmchip[i] = opl2_init_regs[i];
      opl->write(i, opl2_init_regs[i]);
    // }
  }

  // Reset channels
  for(int i = 0; i < 9; i++) {
    chan_offset[i] = chan_start_offset[i];
    chan_pos[i] = 0;
  }

  songpos = 0;
}

/*** private methods *************************************/

inline void CplxPlayer::setregs(unsigned char reg, unsigned char val)
{
  if(fmchip[reg] == val) return;

  fmchip[reg] = val;
  opl->write(reg, val);
}

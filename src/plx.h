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
 * plx.h - PALLADIX Player by Simon Peter <simonpeter303@gmail.com>
 */

#include <binstr.h>

#include "player.h"

class CplxPlayer: public CPlayer
{
 public:
  static CPlayer *factory(Copl *newopl) { return new CplxPlayer(newopl); }

  CplxPlayer(Copl *newopl);
  virtual ~CplxPlayer();

  bool load(const std::string &filename, const CFileProvider &fp);
  virtual bool update();
  virtual void rewind(int subsong = -1);
  float getrefresh() { return 1193182.0f / (speed_scale * speed); }

  std::string gettype() { return std::string("PALLADIX Sound System"); }
  unsigned int getrow() { return songpos; }
  unsigned int getspeed() { return speed_scale; }

 private:
  static const unsigned short	frequency[];
  static const unsigned char	opl2_init_regs[];
  unsigned char			fmchip[0xff];
  unsigned char			*songdata;
  unsigned short		speed;
  unsigned char			type, speed_scale, chan_volume[9];
  unsigned short		chan_start_offset[9], chan_offset[9], chan_pos[9], songpos;
  binisstream			*song;

  inline void setregs(unsigned char reg, unsigned char val);
};

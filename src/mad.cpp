/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999, 2000, 2001, 2002 Simon Peter, <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
  mad.cpp - MAD loader by Riven the Mage <riven@ok.ru>
*/

#include "mad.h"

/* -------- Public Methods -------------------------------- */

bool CmadLoader::load(istream &f)
{
  const unsigned char conv_inst[10] = { 2,1,10,9,4,3,6,5,8,7 };

  int i,j,k,t;

  // 'MAD+' - signed ?
  char id[4];
  f.read(id,4);
  if (strncmp(id,"MAD+",4))
    return false;

  // load instruments
  f.read((char *)instruments,9*sizeof(mad_instrument));

  // fix instruments
  for(i=0;i<9;i++)
    for(j=0;j<10;j++)
      inst[i].data[conv_inst[j]] = instruments[i].data[j];

  // data for ProTracker
  f.ignore(1);
  length = f.get();
  nop = f.get();
  timer = f.get();

  // load tracks
  for(i=0;i<nop;i++)
  {
    for(k=0;k<32;k++)
    {
      for(j=0;j<9;j++)
      {
        // calculate track number
        t = i*9 + j;

        // read event
        unsigned char event = f.get();

        // convert event
        tracks[t][k].note = (event < 0x61) ? event : 0;
        tracks[t][k].inst = (event < 0x61) ? j + 1 : 0;
        tracks[t][k].command = 0;
        tracks[t][k].param1 = 0;
        tracks[t][k].param2 = 0;

        // fix fx
        if (event == 0xFF) // 0xFF: Release note
          tracks[t][k].command = 8;
        if (event == 0xFE) // 0xFE: Pattern Break
          tracks[t][k].command = 13;
      }
    }

    // set 'end of pattern' (due to 32-row-patterns)
    tracks[i*9+8][31].command = 13;

    // save track numbers
    for(j=0;j<9;j++)
      trackord[i][j] = i*9 + j + 1;
  }

  // load order
  f.read((char *)order,length);

  // fix order
  for(i=0;i<length;i++)
    order[i]--;

  // data for ProTracker
  activechan = 0xffff;
  restartpos = 0;
  initspeed = 1;

  rewind(0);

  return true;
}

float CmadLoader::getrefresh()
{
  return (float)timer;
}

std::string CmadLoader::gettype()
{
  return std::string("Mlat Adlib Tracker");
}

std::string CmadLoader::getinstrument(unsigned int n)
{
  return std::string(instruments[n].name,8);
}

unsigned int CmadLoader::getinstruments()
{
  return 9;
}

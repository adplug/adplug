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
  fmc.cpp - FMC loader by Riven the Mage <riven@ok.ru>
*/

#include "fmc.h"

/* -------- Public Methods -------------------------------- */

bool CfmcLoader::load(istream &f)
{
  const unsigned char conv_fx[16] = {0,1,2,3,4,8,255,255,255,255,26,11,12,13,14,15};

  int i,j,t;
  fmc_event event;

  // 'FMC!' - signed ?
  f.read((char *)&header,sizeof(fmc_header));
  if (strncmp(header.id,"FMC!",4))
    return false;

  // load order
  f.read((char *)order,256);

  f.ignore(2);

  // load instruments
  f.read((char *)instruments,32*sizeof(fmc_instrument));

  // load tracks
  t = 0;
  while(f.peek() != EOF)
  {
    for(i=0;i<header.numchan;i++)
    {
      for(j=0;j<64;j++)
      {
        // read event
        f.read((char *)&event,sizeof(fmc_event));

        // convert event
        tracks[t][j].note = event.byte0 & 0x7F;
        tracks[t][j].inst = ((event.byte0 & 0x80) >> 3) + (event.byte1 >> 4) + 1;
        tracks[t][j].command = conv_fx[event.byte1 & 0x0F];
        tracks[t][j].param1 = event.byte2 >> 4;
        tracks[t][j].param2 = event.byte2 & 0x0F;

        // convert fx
        if (tracks[t][j].command == 0x0E) // 0x0E (14): Retrig
          tracks[t][j].param1 = 3;
        if (tracks[t][j].command == 0x1A) // 0x1A (26): Volume Slide
          if (tracks[t][j].param1 > tracks[t][j].param2)
          {
            tracks[t][j].param1 -= tracks[t][j].param2;
            tracks[t][j].param2 = 0;
          }
          else
          {
            tracks[t][j].param2 -= tracks[t][j].param1;
            tracks[t][j].param1 = 0;
          }
      }

      // save track number
      trackord[t/header.numchan][i] = ++t;
    }
  }

  // compute order length
  for(i=0;i<256;i++)
    if (order[i] >= 0xFE)
      length = i;

  // convert instruments
  for(i=0;i<31;i++)
    buildinst(i);

  // data for ProTracker
  activechan = (0xffff >> (16 - header.numchan)) << (16 - header.numchan);
  nop = t / header.numchan;
  restartpos = 0;

  // default volumes
  for(i=0;i<9;i++)
  {
    channel[i].vol1 = 63;
    channel[i].vol2 = 63;
  }

  // and flags
  flags |= MOD_FLAGS_FAUST;

  rewind(0);

  return true;
}

float CfmcLoader::getrefresh()
{
  return 50.0f;
}

std::string CfmcLoader::gettype()
{
  return std::string("Faust Music Creator");
}

std::string CfmcLoader::gettitle()
{
  return std::string(header.title);
}

std::string CfmcLoader::getinstrument(unsigned int n)
{
  return std::string(instruments[n].name);
}

unsigned int CfmcLoader::getinstruments()
{
  return 32;
}

/* -------- Private Methods ------------------------------- */

void CfmcLoader::buildinst(unsigned char i)
{
  inst[i].data[0]   = ((instruments[i].synthesis & 1) ^ 1);
  inst[i].data[0]  |= ((instruments[i].feedback & 7) << 1);

  inst[i].data[3]   = ((instruments[i].mod_attack & 15) << 4);
  inst[i].data[3]  |=  (instruments[i].mod_decay & 15);
  inst[i].data[5]   = ((15 - (instruments[i].mod_sustain & 15)) << 4);
  inst[i].data[5]  |=  (instruments[i].mod_release & 15);
  inst[i].data[9]   =  (63 - (instruments[i].mod_volume & 63));
  inst[i].data[9]  |= ((instruments[i].mod_ksl & 3) << 6);
  inst[i].data[1]   =  (instruments[i].mod_freq_multi & 15);
  inst[i].data[7]   =  (instruments[i].mod_waveform & 3);
  inst[i].data[1]  |= ((instruments[i].mod_sustain_sound & 1) << 5);
  inst[i].data[1]  |= ((instruments[i].mod_ksr & 1) << 4);
  inst[i].data[1]  |= ((instruments[i].mod_vibrato & 1) << 6);
  inst[i].data[1]  |= ((instruments[i].mod_tremolo & 1) << 7);

  inst[i].data[4]   = ((instruments[i].car_attack & 15) << 4);
  inst[i].data[4]  |=  (instruments[i].car_decay & 15);
  inst[i].data[6]   = ((15 - (instruments[i].car_sustain & 15)) << 4);
  inst[i].data[6]  |=  (instruments[i].car_release & 15);
  inst[i].data[10]  =  (63 - (instruments[i].car_volume & 63));
  inst[i].data[10] |= ((instruments[i].car_ksl & 3) << 6);
  inst[i].data[2]   =  (instruments[i].car_freq_multi & 15);
  inst[i].data[8]   =  (instruments[i].car_waveform & 3);
  inst[i].data[2]  |= ((instruments[i].car_sustain_sound & 1) << 5);
  inst[i].data[2]  |= ((instruments[i].car_ksr & 1) << 4);
  inst[i].data[2]  |= ((instruments[i].car_vibrato & 1) << 6);
  inst[i].data[2]  |= ((instruments[i].car_tremolo & 1) << 7);

  inst[i].slide     =   instruments[i].pitch_shift;
}

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2002 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * adtrack.cpp - Adlib Tracker 1.0 Loader by Simon Peter <dn.tlp@gmx.net>
 *
 * NOTES:
 * The original Adlib Tracker 1.0 is behaving a little different from the
 * official spec: The 'octave' integer from the instrument file is stored
 * "minus 1" from the actual value, underflowing from 0 to 0xffff.
 */

#include <stdlib.h>
#include <string.h>
#include <fstream.h>

#include "adtrack.h"
#include "debug.h"

/*** Constants ***/

const unsigned short CadtrackLoader::my_notetable[12] =
  {0x16b,0x181,0x198,0x1b0,0x1ca,0x1e5,0x202,0x220,0x241,0x263,0x287,0x2ae};

/*** Public methods ***/

CPlayer *CadtrackLoader::factory(Copl *newopl)
{
  CadtrackLoader *p = new CadtrackLoader(newopl);
  return p;
}

bool CadtrackLoader::load(istream &f, const char *filename)
{
  char *instfilename, note[2];
  unsigned short rwp;
  unsigned char chp, octave, pnote = 0;
  AdTrackInst myinst;

  // file validation
  if(strlen(filename) < 4 || stricmp(filename+strlen(filename)-4,".sng"))
    return false;
  f.seekg(0,ios::end); if(f.tellg() != 36000) return false; f.seekg(0);
  LogWrite("CadtrackLoader::load(,\"%s\"): Valid .sng file. Checking for .ins file...\n",filename);

  // check for instruments file
  instfilename = (char *)malloc(strlen(filename)+1);
  strcpy(instfilename,filename);
  strcpy(strrchr(instfilename,'.'),".ins");
  ifstream instf(instfilename, ios::in | ios::binary);
  free(instfilename);
  if(!instf.is_open()) return false;
  instf.seekg(0,ios::end); if(instf.tellg() != 468) return false; instf.seekg(0);
  LogWrite("CadtrackLoader::load(,,): Valid .ins file found! Loading...\n");

  // give CmodPlayer a hint on what we're up to
  realloc_patterns(1,1000,9); realloc_instruments(9); realloc_order(1);
  init_trackord(); flags = NoKeyOn; // init_notetable(my_notetable);
  (*order) = 0; length = 1; restartpos = 0; bpm = 120; initspeed = 3;

  // load instruments from instruments file
  for(unsigned char i=0;i<9;i++) {
    instf.read((char *)&myinst,sizeof(AdTrackInst));
    convert_instrument(i, &myinst);
  }

  // load file
  for(rwp=0;rwp<1000;rwp++)
    for(chp=0;chp<9;chp++) {
      f.read(note,2); octave = f.get(); f.ignore();	// read next record
      switch(*note) {
      case 'C': if(note[1] == '#') pnote = 2; else pnote = 1; break;
      case 'D': if(note[1] == '#') pnote = 4; else pnote = 3; break;
      case 'E': pnote = 5; break;
      case 'F': if(note[1] == '#') pnote = 7; else pnote = 6; break;
      case 'G': if(note[1] == '#') pnote = 9; else pnote = 8; break;
      case 'A': if(note[1] == '#') pnote = 11; else pnote = 10; break;
      case 'B': pnote = 12; break;
      case '\0':
	if(note[1] == '\0') tracks[chp][rwp].note = 127; else return false;
	break;
      default: return false;
      }
      if((*note) != '\0') {
	tracks[chp][rwp].note = pnote + (octave * 12);
	tracks[chp][rwp].inst = chp + 1;
      }
    }

  rewind(0);
  return true;
}

float CadtrackLoader::getrefresh()
{
  return 18.2f;
}

/*** Private methods ***/

void CadtrackLoader::convert_instrument(unsigned int n, AdTrackInst *i)
{
  // Carrier "Amp Mod / Vib / Env Type / KSR / Multiple" register
  inst[n].data[2] = i->op[Carrier].appampmod ? 1 << 7 : 0;
  inst[n].data[2] += i->op[Carrier].appvib ? 1 << 6 : 0;
  inst[n].data[2] += i->op[Carrier].maintsuslvl ? 1 << 5 : 0;
  inst[n].data[2] += i->op[Carrier].keybscale ? 1 << 4 : 0;
  inst[n].data[2] += (i->op[Carrier].octave + 1) & 0xffff; // Bug in original Tracker
  // Modulator...
  inst[n].data[1] = i->op[Modulator].appampmod ? 1 << 7 : 0;
  inst[n].data[1] += i->op[Modulator].appvib ? 1 << 6 : 0;
  inst[n].data[1] += i->op[Modulator].maintsuslvl ? 1 << 5 : 0;
  inst[n].data[1] += i->op[Modulator].keybscale ? 1 << 4 : 0;
  inst[n].data[1] += (i->op[Modulator].octave + 1) & 0xffff; // Bug in original tracker

  // Carrier "Key Scaling / Level" register
  inst[n].data[10] = (i->op[Carrier].freqrisevollvldn & 3) << 6;
  inst[n].data[10] += i->op[Carrier].softness & 63;
  // Modulator...
  inst[n].data[9] = (i->op[Modulator].freqrisevollvldn & 3) << 6;
  inst[n].data[9] += i->op[Modulator].softness & 63;

  // Carrier "Attack / Decay" register
  inst[n].data[4] = (i->op[Carrier].attack & 0x0f) << 4;
  inst[n].data[4] += i->op[Carrier].decay & 0x0f;
  // Modulator...
  inst[n].data[3] = (i->op[Modulator].attack & 0x0f) << 4;
  inst[n].data[3] += i->op[Modulator].decay & 0x0f;

  // Carrier "Release / Sustain" register
  inst[n].data[6] = (i->op[Carrier].release & 0x0f) << 4;
  inst[n].data[6] += i->op[Carrier].sustain & 0x0f;
  // Modulator...
  inst[n].data[5] = (i->op[Modulator].release & 0x0f) << 4;
  inst[n].data[5] += i->op[Modulator].sustain & 0x0f;

  // Channel "Feedback / Connection" register
  inst[n].data[0] = (i->op[Carrier].feedback & 7) << 1;

  // Carrier/Modulator "Wave Select" registers
  inst[n].data[8] = i->op[Carrier].waveform & 3;
  inst[n].data[7] = i->op[Modulator].waveform & 3;
}

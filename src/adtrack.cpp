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
 * adtrack.cpp - Adlib Tracker 1.0 Loader by Simon Peter <dn.tlp@gmx.net>
 */

#include <stdlib.h>
#include <string.h>
#include <fstream.h>

#include "adtrack.h"
#include "debug.h"

CPlayer *CadtrackLoader::factory(Copl *newopl)
{
  CamdLoader *p = new CamdLoader(newopl);
  return p;
}

bool CadtrackLoader::load(istream &f, const char *filename)
{
  char *instfilename, note[2];
  unsigned short rwp;
  unsigned char chp, octave;

  // file validation
  if(strlen(filename) < 4 || stricmp(filename+strlen(filename)-4,".sng"))
    return false;
  f.seekg(0,ios::end); if(f.tellg() != 36000) return false; f.seekg(0);

  // check for instruments file
  instfilename = (char *)malloc(strlen(filename)+1);
  strcpy(instfilename,filename);
  strcpy(strrchr(instfilename,'.'),".ins");
  ifstream instf(instfilename, ios::in | ios::binary);
  free(instfilename);
  if(!instf.is_open()) return false;

  // give CmodPlayer a hint on what we're up to
  realloc_patterns(1,1000,9); realloc_instruments(9); realloc_order(1);
  init_trackord();

  // load instruments from instruments file
  

  // load file
  for(rwp=0;rwp<1000;rwp++)
    for(chp=0;chp<9;chp++) {
      f.read(note,2); octave = f.get(); f.ignore();	// read next record
      switch(note) {
      }
      tracks[chp][rwp] = 
    }

  rewind(0);
  return true;
}

float CadtrackLoader::getrefresh()
{
  return 18.2f;
}

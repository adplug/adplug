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
 * playertest.cpp - Test AdPlug replayers, by Simon Peter <dn.tlp@gmx.net>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string>

#include "../src/adplug.h"
#include "../src/diskopl.h"

unsigned long filesize(FILE *f)
{
  long	fpos = ftell(f), fsize;
  fseek(f, 0, SEEK_END);
  fsize = ftell(f);
  fseek(f, fpos, SEEK_SET);
  return fsize;
}

long read_file(char **mem, const std::string filename)
{
  FILE	*f = fopen(filename.c_str(), "rb");
  if(!f) return -1;
  long	fsize = filesize(f);

  (*mem) = (char *)malloc(fsize);
  fread(*mem, fsize, 1, f);
  fclose(f);

  return fsize;
}

bool testplayer(const std::string filename)
{
  CDiskopl	*opl = new CDiskopl(filename + ".test.raw");
  CPlayer	*p = CAdPlug::factory(filename, opl);
  bool		retval = true;

  if(!p) { delete opl; return false; }

  // Output file information
  std::cout << "Testing format: " << p->gettype() << std::endl;

  // Write whole file to disk
  while(p->update())
    opl->update(p);

  delete p;
  delete opl;

  // Compare with original
  char	*f1, *f2;
  long	fsize1 = read_file(&f1, filename + ".orig.raw");
  long	fsize2 = read_file(&f2, filename + ".test.raw");

  if(fsize1 == fsize2) {
    for(long i = 0; i < fsize1; i++)
      if(f1[i] != f2[i]) {
	retval = false;
	break;
      }
  } else
    retval = false;

  if(fsize1 != -1) free(f1);
  if(fsize2 != -1) free(f2);
  remove(std::string(filename + ".test.raw").c_str());
  return retval;
}

int main(int argc, char *argv[])
{
  if(!testplayer("SONG1.sng")) return EXIT_FAILURE;	// Adlib Tracker
  if(!testplayer("2001.MKJ")) return EXIT_FAILURE;	// MK-Jamz
  if(!testplayer("ADAGIO.DFM")) return EXIT_FAILURE;	// 
  if(!testplayer("adlibsp.s3m")) return EXIT_FAILURE;	// 
  if(!testplayer("ALLOYRUN.RAD")) return EXIT_FAILURE;	// 
  if(!testplayer("ARAB.BAM")) return EXIT_FAILURE;	// 
  if(!testplayer("BEGIN.KSM")) return EXIT_FAILURE;	// 
  if(!testplayer("BOOTUP.M")) return EXIT_FAILURE;	// 
  if(!testplayer("CHILD1.XSM")) return EXIT_FAILURE;	// 
  if(!testplayer("DTM-TRK1.DTM")) return EXIT_FAILURE;	// 
  if(!testplayer("fdance03.dmo")) return EXIT_FAILURE;	// 
  if(!testplayer("ice_think.sci")) return EXIT_FAILURE;	// 
  if(!testplayer("inc.raw")) return EXIT_FAILURE;	// 
  //  if(!testplayer("loudness.lds")) return EXIT_FAILURE;	// 
  if(!testplayer("MARIO.A2M")) return EXIT_FAILURE;	// 
  if(!testplayer("mi2_big_tree1.laa")) return EXIT_FAILURE;	// 
  if(!testplayer("michaeld.cmf")) return EXIT_FAILURE;	// 
  if(!testplayer("PLAYMUS1.SNG")) return EXIT_FAILURE;	// 
  if(!testplayer("rat.xad")) return EXIT_FAILURE;	// 
  if(!testplayer("REVELAT.SNG")) return EXIT_FAILURE;	// 
  if(!testplayer("SAILOR.CFF")) return EXIT_FAILURE;	// 
  if(!testplayer("samurai.dro")) return EXIT_FAILURE;	// 
  if(!testplayer("SCALES.SA2")) return EXIT_FAILURE;	// 
  if(!testplayer("SMKEREM.HSC")) return EXIT_FAILURE;	// 
  if(!testplayer("TOCCATA.MAD")) return EXIT_FAILURE;	// 
  if(!testplayer("TUBES.SAT")) return EXIT_FAILURE;	// 
  if(!testplayer("TU_BLESS.AMD")) return EXIT_FAILURE;	// 
  if(!testplayer("VIB_VOL3.D00")) return EXIT_FAILURE;	// 
  if(!testplayer("WONDERIN.IMF")) return EXIT_FAILURE;	// 

  return EXIT_SUCCESS;
}

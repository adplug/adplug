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

/***** Local variables *****/

// List of all filenames to test
static const char *filelist[] = {
  "SONG1.sng",		// Adlib Tracker
  "2001.MKJ",		// MK-Jamz
  "ADAGIO.DFM",		// Digital-FM
  "adlibsp.s3m",	// Scream Tracker 3
  "ALLOYRUN.RAD",	// Reality AdLib Tracker
  "ARAB.BAM",		// Bob's AdLib Music
  "BEGIN.KSM",		// Ken Silverman
  "BOOTUP.M",		// Ultima 6
  "CHILD1.XSM",		// eXtra Simple Music
  "DTM-TRK1.DTM",	// DeFy Adlib Tracker
  "fdance03.dmo",	// TwinTrack
  "ice_think.sci",	// Sierra */
  "inc.raw",		// RAW
  //  "loudness.lds",	// Loudness
  "MARIO.A2M",		// AdLib Tracker 2
  "mi2_big_tree1.laa",	// LucasArts
  "michaeld.cmf",	// Creative Music Format
  "PLAYMUS1.SNG",	// SNGPlay
  "rat.xad",		// XAD
  "REVELAT.SNG",	// Faust Music Creator
  "SAILOR.CFF",		// Boomtracker
  "samurai.dro",	// DOSBox
  "SCALES.SA2",		// Surprise! Adlib Tracker 2
  "SMKEREM.HSC",	// HSC-Tracker
  "TOCCATA.MAD",	// Mlat Adlib Tracker
  "TUBES.SAT",		// Surprise! Adlib Tracker
  "TU_BLESS.AMD",	// AMUSIC
  "VIB_VOL3.D00",	// EdLib Packed
  "WONDERIN.IMF",	// Apogee
  NULL
};

/***** Local functions *****/

static unsigned long filesize(FILE *f)
  /* Returns the file size of file f. */
{
  long	fpos = ftell(f), fsize;
  fseek(f, 0, SEEK_END);
  fsize = ftell(f);
  fseek(f, fpos, SEEK_SET);
  return fsize;
}

static long read_file(char **mem, const std::string filename)
  /*
   * Reads file 'filename' into memory area 'mem', which will be allocated and
   * must be freed later, and returns its size.
   */
{
  FILE	*f = fopen(filename.c_str(), "rb");
  if(!f) return -1;
  long	fsize = filesize(f);

  (*mem) = (char *)malloc(fsize);
  fread(*mem, fsize, 1, f);
  fclose(f);

  return fsize;
}

static bool testplayer(const std::string filename)
  /*
   * Tests playback of file 'filename' by comparing its RAW output with a
   * prerecorded original and returns true if they match, false otherwise.
   */
{
  std::string	fn = std::string(getenv("srcdir")) + "/" + filename;
  CDiskopl	*opl = new CDiskopl(filename + ".test.raw");
  CPlayer	*p = CAdPlug::factory(fn, opl);
  bool		retval = true;

  if(!p) { delete opl; return false; }

  // Output file information
  std::cout << "Testing format: " << p->gettype() << " - ";

  // Write whole file to disk
  while(p->update())
    opl->update(p);

  delete p;
  delete opl;

  // Compare with original
  char	*f1, *f2;
  long	fsize1 = read_file(&f1, fn + ".orig.raw");
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
  if(retval) {
    std::cout << "OK\n";
    remove(std::string(filename + ".test.raw").c_str());
  } else
    std::cout << "FAIL\n";
  return retval;
}

/***** Main program *****/

int main(int argc, char *argv[])
{
  unsigned int	i;
  bool		retval = true;

  // Try all files one by one
  for(i = 0; filelist[i] != NULL; i++)
    if(!testplayer(filelist[i]))
      retval = false;

  return retval ? EXIT_SUCCESS : EXIT_FAILURE;
}

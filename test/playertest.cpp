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
#include "../src/opl.h"

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
  "ice_think.sci",	// Sierra
  "inc.raw",		// RAW
  "loudness.lds",	// Loudness
  "MARIO.A2M",		// AdLib Tracker 2
  "mi2_big_tree1.laa",	// LucasArts
  "michaeld.cmf",	// Creative Music Format
  "PLAYMUS1.SNG",	// SNGPlay
  "rat.xad",		// xad: rat
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
  "bmf1_2.xad",		// xad: BMF
  "flash.xad",		// xad. flash
  "HIP_D.ROL",		// Visual Composer
  "hybrid.xad", 	// xad: hybrid
  "hyp.xad",		// xad: hyp
  "psi1.xad",		// xad: PSI
  NULL
};

/***** Testopl *****/

class Testopl: public Copl
{
public:
  Testopl(const std::string filename)
  {
    f = fopen(filename.c_str(), "w");
  }

  virtual ~Testopl()
  {
    if(f) fclose(f);
  }

  void update(CPlayer *p)
  {
    if(!f) return;
    fprintf(f, "r%.2f\n", p->getrefresh());
  }

  // template methods
  void write(int reg, int val)
  {
    if(reg > 255 || val > 255)
      std::cerr << "Warning: The player is writing data out of range! (reg = "
		<< reg << ", val = " << val << ")\n";
    if(!f) return;
    fprintf(f, "%u <- %u\n", reg, val);
  }

  void init()
  {
    if(!f) return;
    fprintf(f, "init\n");
  }

private:
  FILE	*f;
};

/***** Local functions *****/

static bool diff(const std::string fn1, const std::string fn2)
  /*
   * Compares files 'fn1' and 'fn2' line by line and returns true if they are
   * equal or false otherwise. A line is at most 79 characters in length or the
   * comparison will fail.
   */
{
  FILE	*f1, *f2;
  bool	retval = true;

  // open both files
  if(!(f1 = fopen(fn1.c_str(), "r"))) return false;
  if(!(f2 = fopen(fn2.c_str(), "r"))) { fclose(f1); return false; }

  // compare both files line by line
  char	*s1 = (char *)malloc(80), *s2 = (char *)malloc(80);
  while(!(feof(f1) || feof(f2))) {
    fgets(s1, 80, f1);
    fgets(s2, 80, f2);
    if(strncmp(s1, s2, 79)) {
      retval = false;
      break;
    }
  }
  free(s1), free(s2);
  if(feof(f1) != feof(f2))
    retval = false;

  // close both files
  fclose(f1), fclose(f2);
  return retval;
}

static bool testplayer(const std::string filename)
  /*
   * Tests playback of file 'filename' by comparing its RAW output with a
   * prerecorded original and returns true if they match, false otherwise.
   */
{
  std::string	fn = std::string(getenv("srcdir")) + "/" + filename;
  Testopl	*opl = new Testopl(filename + ".test");
  CPlayer	*p = CAdPlug::factory(fn, opl);

  if(!p) {
    std::cout << "Error loading: " << fn << std::endl;
    delete opl; return false;
  }

  // Output file information
  std::cout << "Testing format: " << p->gettype() << " - ";

  // Write whole file to disk
  while(p->update())
    opl->update(p);

  delete p;
  delete opl;

  if(diff(fn + ".ref", filename + ".test")) {
    std::cout << "OK\n";
    remove(std::string(filename + ".test").c_str());
    return true;
  } else {
    std::cout << "FAIL\n";
    return false;
  }
}

/***** Main program *****/

int main(int argc, char *argv[])
{
  unsigned int	i;
  bool		retval = true;

  // Try all files one by one
  if(argc > 1) {
    for(i = 1; i < argc; i++)
      if(!testplayer(argv[i]))
	retval = false;
  } else
    for(i = 0; filelist[i] != NULL; i++)
      if(!testplayer(filelist[i]))
	retval = false;

  return retval ? EXIT_SUCCESS : EXIT_FAILURE;
}

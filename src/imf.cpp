/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * imf.cpp - IMF Player by Simon Peter <dn.tlp@gmx.net>
 *
 * FILE FORMAT:
 * There seem to be 2 different flavors of IMF formats out there. One version
 * contains just the raw IMF music data. In this case, the first word of the
 * file is always 0 (because the music data starts this way). This is already
 * the music data! So read in the entire file and play it.
 *
 * If this word is greater than 0, it specifies the size of the following
 * song data in bytes. In this case, the file has a footer that contains
 * arbitrary infos about it. Mostly, this is plain ASCII text with some words
 * of the author. Read and play the specified amount of song data and display
 * the remaining data as ASCII text.
 */

#include <string.h>

#include "imf.h"
#include "database.h"

/*** public methods *************************************/

CPlayer *CimfPlayer::factory(Copl *newopl)
{
  return new CimfPlayer(newopl);
}

bool CimfPlayer::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f = fp.open(filename); if(!f) return false;
  unsigned short fsize;
  unsigned long flsize;
  unsigned int i;

  // file validation section (actually just an extension check)
  if(!fp.extension(filename, ".imf")) { fp.close(f); return false; }

  // load section
  fsize = f->readInt(2);	// try to load music data size
  flsize = fp.filesize(f);
  if(!fsize) {		// footerless file (raw music data)
    f->seek(0);
    size = flsize / 4;
  } else		// file has got footer
    size = fsize / 4;

  data = new Sdata[size];
  for(i=0; i<size; i++) {
    data[i].reg = f->readInt(1); data[i].val = f->readInt(1);
    data[i].time = f->readInt(2);
  }
  if(fsize && (fsize < flsize - 2)) {	// read footer, if any
    unsigned long footerlen = flsize - fsize - 2;
    footer = new char[footerlen + 1];
    f->readString(footer,footerlen);
    footer[footerlen] = '\0';	// Make ASCIIZ string
  }

  rate = getrate(f);
  fp.close(f);
  rewind(0);
  return true;
}

bool CimfPlayer::update()
{
	do {
		opl->write(data[pos].reg,data[pos].val);
		del = data[pos].time;
		pos++;
	} while(!del && pos < size);

	if(pos >= size) {
		pos = 0;
		songend = true;
	}
	else timer = rate / (float)del;

	return !songend;
}

void CimfPlayer::rewind(int subsong)
{
	pos = 0; del = 0; timer = rate; songend = false;
	opl->init(); opl->write(1,32);	// go to OPL2 mode
}

/*** private methods *************************************/

float CimfPlayer::getrate(binistream *f)
{
  if(!db) return 700.0f;	// Database offline
  f->seek(0, binio::Set);
  CClockRecord *record = (CClockRecord *)db->search(CAdPlugDatabase::CKey(*f));
  if(!record || record->type != CAdPlugDatabase::CRecord::ClockSpeed)
    return 700.0f;
  else
    return record->clock;
}

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2005 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * raw.c - RAW Player by Simon Peter <dn.tlp@gmx.net>
 */

/*
 * Copyright (c) 2015 - 2017 Wraithverge <liam82067@yahoo.com>
 * - Preliminary support for displaying arbitrary Tag data. (2015)
 * - Realigned to Tabs. (2017)
 * - Added Member pointers. (2017)
 * - Finalized Tag support. (2017)
 */

#include <cstring>
#include "raw.h"

/*** public methods *************************************/

CPlayer *CrawPlayer::factory(Copl *newopl)
{
  return new CrawPlayer(newopl);
}

bool CrawPlayer::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f = fp.open(filename);
  if (!f) return false;

  char id[8];
  unsigned long i = 0;

  // file validation section
  f->readString(id, 8);
  if (strncmp(id,"RAWADATA",8)) { fp.close (f); return false; }

  // load section
  this->clock = f->readInt(2); // clock speed
  this->length = (fp.filesize(f) - 10) / 2; // Wraithverge: total data-size.
  this->data = new Tdata [this->length];
  bool tagdata = false;

  for (i = 0; i < this->length; i++) {
    // Do not store tag data in track data.
    this->data[i].param = (tagdata ? 0xFF : f->readInt(1));
    this->data[i].command = (tagdata ? 0xFF : f->readInt(1));

    // Continue trying to stop at the RAW EOF data marker.
    if (!tagdata && this->data[i].param == 0xFF && this->data[i].command == 0xFF) {
      if ((unsigned char)f->readInt(1) == 0x1A) {
        // Tag marker found.
        tagdata = true;
      } else {
        // This is not tag marker, revert.
        f->seek(-1, binio::Add);
      }
    }
  }

  if (tagdata)
  {
    // The arbitrary Tag Data section begins here.

    // "title" is maximum 40 characters long.
    f->readString(title, 40, 0);

    // Safety check, in case "title" is empty, set a
    // null-terminator as placeholder.
    if (strlen(title) == 0) memset(title, 0, 1);

    // Skip "author" if Tag marker byte is missing, but first
    // set a null-terminator as placeholder.
    if (f->readInt(1) != 0x1B) {
      memset(author, 0, 1);
      goto desc_section;
    }

    // "author" is maximum 40 characters long.
    f->readString(author, 40, 0);

    // Safety check, in case "author" is empty, set a
    // null-terminator as placeholder.
    if (strlen(author) == 0) memset(author, 0, 1);

    // Skip "desc" if Tag marker byte is missing, but first
    // set a null-terminator as placeholder.
    if (f->readInt(1) != 0x1C) {
      memset(desc, 0, 1);
      goto end_section;
    }
    else goto desc_section;

desc_section:
    // "desc" is now maximum 1023 characters long (it was 140).
    f->readString(desc, 1023, 0);

    // Safety check, in case "desc" is empty, set a
    // null-terminator as placeholder.
    if (strlen(desc) == 0) {
      memset(desc, 0, 1);
      goto end_section;
    }
    else goto end_section;
  }

end_section:
  fp.close(f);
  rewind(0);
  return true;
}

bool CrawPlayer::update()
{
  bool setspeed = 0;

  if (this->pos >= this->length) return false;

  if (this->del) {
    del--;
    return !songend;
  }

  do {
    setspeed = false;

    switch(this->data[this->pos].command) {
    case 0:
      this->del = this->data[this->pos].param - 1;
      break;

    case 2:
      if (!this->data[this->pos].param) {
        pos++;
        this->speed = this->data[this->pos].param + (this->data[this->pos].command << 8);
        setspeed = true;
      } else
        opl->setchip(this->data[this->pos].param - 1);
      break;

    case 0xff:
      if (this->data[this->pos].param == 0xff) {
        rewind(0); // auto-rewind song
        songend = true;
        return !songend;
      }
      break;

    default:
      opl->write(this->data[this->pos].command, this->data[this->pos].param);
      break;
    }
  } while (this->data[this->pos++].command || setspeed);

  return !songend;
}

void CrawPlayer::rewind(int subsong)
{
  pos = del = 0; speed = clock = 0; songend = false;
  opl->init(); opl->write(1, 32);	// go to 9 channel mode
}

float CrawPlayer::getrefresh()
{
  // timer oscillator speed / wait register = clock frequency
  return 1193180.0 / (speed ? speed : 0xffff);
}

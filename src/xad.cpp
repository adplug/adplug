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
  xad.cpp - XAD shell player by Riven the Mage <riven@ok.ru>
*/

// Redefine this, if you want the debug output to go elsewhere.
// Under DOS/Windows, using "CON:" as filename will log to the (debug) console.
#ifdef _DEBUG
#define XAD_DEBUG_FILE	"xad.log"
#endif

#include "xad.h"
#include "debug.h"

/* -------- Public Methods -------------------------------- */

CxadPlayer::CxadPlayer(Copl * newopl) : CPlayer(newopl)
{
#ifdef _DEBUG
  LogOpen(XAD_DEBUG_FILE);
#endif

  tune = 0;
}

CxadPlayer::~CxadPlayer()
{
  if (tune)
    delete [] tune;

#ifdef _DEBUG
  LogClose();
#endif
}

bool CxadPlayer::load(istream &f)
{
  bool ret = false;

  // 'XAD!' - signed ?
  f.read((char *)&xad,sizeof(xad_header));
  if (*(unsigned long *)xad.id != 0x21444158)
    return false;

  // get file size
  f.seekg(0,ios::end);
  tune_size = f.tellg();
  f.seekg(sizeof(xad_header));
  tune_size -= sizeof(xad_header);

  // load()
  tune = new unsigned char [tune_size];
  f.read((char *)tune, tune_size);

  ret = xadplayer_load(f);

  if (ret)
    rewind(0);

  return ret;
}

void CxadPlayer::rewind(unsigned int subsong)
{
  opl->init();

  plr.speed = xad.speed;
  plr.speed_counter = 1;
  plr.playing = 1;
  plr.looping = 0;

  // rewind()
  xadplayer_rewind(subsong);

#ifdef _DEBUG
  LogWrite("-----------\n");
#endif
}

bool CxadPlayer::update()
{
  if (--plr.speed_counter)
    goto update_end;

  plr.speed_counter = plr.speed;

  // update()
  xadplayer_update();

update_end:
  return (plr.playing && (!plr.looping));
}

#ifdef WIN32
#pragma warning(disable:4715)
#endif

float CxadPlayer::getrefresh()
{
  return xadplayer_getrefresh();
}

std::string CxadPlayer::gettype()
{
  return xadplayer_gettype();
}

#ifdef WIN32
#pragma warning(default:4715)
#endif

std::string CxadPlayer::gettitle()
{
  return xadplayer_gettitle();
}

std::string CxadPlayer::getauthor()
{
  return xadplayer_getauthor();
}

std::string CxadPlayer::getinstrument(unsigned int i)
{
  return xadplayer_getinstrument(i);
}

unsigned int CxadPlayer::getinstruments()
{
  return xadplayer_getinstruments();
}

/* -------- Protected Methods ------------------------------- */

void CxadPlayer::opl_write(int reg, int val)
{
  adlib[reg] = val;
#ifdef _DEBUG
  LogWrite("[ %02X ] = %02X\n",reg,val);
#endif
  opl->write(reg,val);
}

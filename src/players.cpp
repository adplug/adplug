/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
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
 * players.h - Players enumeration, by Simon Peter <dn.tlp@gmx.net>
 */

#include "players.h"

/***** CPlayerDesc *****/

CPlayerDesc::CPlayerDesc()
  : factory(0)
{
}

CPlayerDesc::CPlayerDesc(Factory f, std::string type, std::string ext)
  : factory(f), filetype(type), extension(ext)
{
}

CPlayerDesc::~CPlayerDesc()
{
}

/***** CPlayers *****/

const CPlayerDesc *CPlayers::lookup_extension(const std::string &extension) const
{
  const char *ext = extension.c_str();
  const_iterator i;

  for(i = begin(); i != end(); i++)
    if(!stricmp(ext, (*i)->extension.c_str()))
      return *i;

  return 0;
}

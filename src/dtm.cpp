//
// alpha version. do not compile.
//
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
  dtm.cpp - DTM loader by Riven the Mage <riven@ok.ru>
*/

#include "dtm.h"

/* -------- Public Methods -------------------------------- */

bool CdtmLoader::load(istream &f)
{
  const unsigned char conv_inst[11] = { 2,1,10,9,4,3,6,5,0,8,7 };

  int i,j,t;

  // signature exists ?
  f.read((char *)&header,sizeof(dtm_header));
  if (strncmp(header.id,"DeFy DTM ",9))
    return false;

  // good version ?
  if (header.version != 0x10)
    return false;

  header.numinst++;

  unsigned char bufstr[80];
  unsigned char bufstr_length;

  // load description
  memset(desc,0,80*16);
  for(i=0;i<16;i++)
  {
    // get line length
    bufstr_length = f.get();

    // read line
    if (bufstr_length)
    {
      f.read(bufstr,bufstr_length);

      for(j=0;j<bufstr_length;j++)
        if (!bufstr[i])
          bufstr[i] = 0x20;

      bufstr[bufstr_length] = 0;

      strcat(desc,bufstr);
    }

    strcat(desc,"\n");
  }

  memset(instruments, 0, sizeof(instruments));

  // load instruments
  for(i=0;i<header.numinst;i++)
  {
    f.read(instruments[i].name, f.get());
    f.read(instruments[i].data,12);
  }

  // fix instruments
  for(i=0;i<header.numinst;i++)
    for(j=0;j<11;j++)
      inst[i].data[conv_inst[j]] = instruments[i].data[j];

  // load order
  f.read(order,100);

  // compute order length
  for(i=0;((i < 100) && (order[i] < 0xFE));i++)
    length=i+1;

  // load order
  f.read(order,256);

  f.ignore(2);

  // load instruments
  f.read((char *)instruments,32*sizeof(fmc_instrument));







}




float CdtmLoader::getrefresh()
{
  return 18.2f;
}

std::string CdtmLoader::gettype()
{
  return std::string("DeFy Adlib Tracker");
}

std::string CdtmLoader::gettitle()
{
  return std::string(header.title);
}

std::string CdtmLoader::getauthor()
{
  return std::string(header.author);
}

std::string CdtmLoader::getdesc()
{
  return std::string(desc);
}

std::string CdtmLoader::getinstrument(unsigned int n)
{
  return std::string(instruments[n].name);
}

unsigned int CdtmLoader::getinstruments()
{
  return header.numinst;
}

/* -------- Private Methods ------------------------------- */

void CdtmLoader::unpackpat(BYTE *inbuf, DWORD inbuflen, BYTE *outbuf)
{
  DWORD i,j=0;
  BYTE byte_buf, byte_count;

  // RLE decoder
  for(i=0;i<inbuflen;i++)
  {
    if ((inbuf[i] & 0xF0) == 0xD0)
    {
      byte_count = inbuf[i] & 0x0F;
      byte_buf   = inbuf[++i];

      for(j=0;j<byte_count;j++)
        *outbuf++ = byte_buf;
    }
    else
      *outbuf++ = inbuf[i];
  }
}

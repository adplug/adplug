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
  dmo.cpp - TwinTeam loader by Riven the Mage <riven@ok.ru>
*/

#include "dmo.h"

/* -------- Public Methods -------------------------------- */

CPlayer *CdmoLoader::factory(Copl *newopl)
{
  CdmoLoader *p = new CdmoLoader(newopl);
  return p;
}

bool CdmoLoader::load(istream &f, const char *filename)
{
	rewind(0);

	return true;	
}

std::string CdmoLoader::gettype()
{
	return std::string("TwinTeam Packed S3M");
}

/* -------- Private Methods ------------------------------- */

WORD CdmoLoader::dmo_unpacker::brand(WORD range)
{
	WORD ax,bx,cx,dx;

	ax = LOWORD(bseed);
	bx = HIWORD(bseed);
	cx = ax;
	ax = LOWORD(cx * 0x8405);
	dx = HIWORD(cx * 0x8405);
	cx <<= 3;
	cx = (((HIBYTE(cx) + LOBYTE(cx)) & 0xFF) << 8) + LOBYTE(cx);
	dx += cx;
	dx += bx;
	bx <<= 2;
	dx += bx;
	dx = (((HIBYTE(dx) + LOBYTE(bx)) & 0xFF) << 8) + LOBYTE(dx);
	bx <<= 5;
	dx = (((HIBYTE(dx) + LOBYTE(bx)) & 0xFF) << 8) + LOBYTE(dx);
	ax += 1;
	if (!ax) dx += 1;

	bseed = (dx << 16) + ax;

	return HIWORD(HIWORD(LOWORD(bseed) * range) + HIWORD(bseed) * range);
}

bool CdmoLoader::dmo_unpacker::decrypt(char *buf, long len)
{
	DWORD seed = 0;

	bseed = *(DWORD *)&buf[0];

	for (int i=0;i<(*(WORD *)&buf[4]+1);i++)
		seed += brand(0xffff);

	bseed = seed ^ *(DWORD *)&buf[6];

	if ((*(WORD *)&buf[10]) != brand(0xffff))
		return false;

	for (i=0;i<(len-12);i++)
		buf[12+i] ^= brand(0x100);

	*(WORD *)&buf[len-2] = 0;

	return true;
}

WORD CdmoLoader::dmo_unpacker::unpack_block(char *ibuf, long ilen, char *obuf)
{
	BYTE code,par1,par2;
	WORD ax,bx,cx;

	char *ipos = ibuf;
	char *opos = obuf;

	// LZ??
	while (ipos - ibuf < ilen)
	{
		code = *ipos++;

		// 00xxxxxx: copy (xxxxxx + 1) bytes
		if ((code >> 6) == 0)
		{
			cx = (code & 0x3F) + 1;

			for (int i=0;i<cx;i++)
				*opos++ = *ipos++;

			continue;
		}

		// 01xxxxxx xxxyyyyy: copy (Y + 3) bytes from (X + 1)
		if ((code >> 6) == 1)
		{
			par1 = *ipos++;

			ax = ((code & 0x3F) << 3) + ((par1 & 0xE0) >> 5) + 1;
			cx = (par1 & 0x1F) + 3;

			for(int i=0;i<cx;i++)
				*opos++ = *(opos - ax);

			continue;
		}

		// 10xxxxxx xyyyzzzz: copy (Y + 3) bytes from (X + 1); copy Z bytes
		if ((code >> 6) == 2)
		{
			par1 = *ipos++;

			ax = ((code & 0x3F) << 1) + (par1 >> 7) + 1;
			cx = ((par1 & 0x70) >> 4) + 3;
			bx = par1 & 0x0F;

			for(int i=0;i<cx;i++)
				*opos++ = *(opos - ax);

			for (i=0;i<bx;i++)
				*opos++ = *ipos++;

			continue;
		}

		// 11xxxxxx xxxxxxxy yyyyzzzz: copy (Y + 4) from X; copy Z bytes
		if ((code >> 6) == 3)
		{
			par1 = *ipos++;
			par2 = *ipos++;

			bx = ((code & 0x3F) << 7) + (par1 >> 1);
			cx = ((par1 & 0x01) << 4) + (par2 >> 4) + 4;
			ax = par2 & 0x0F;

			for(int i=0;i<cx;i++)
				*opos++ = *(opos - bx);

			for (i=0;i<ax;i++)
				*opos++ = *ipos++;

			continue;
		}
	}

	return opos - obuf;
}

long CdmoLoader::dmo_unpacker::unpack(char *ibuf, long ilen, char *obuf)
{
	long olen = 0;

	if (!decrypt(ibuf,ilen))
		return 0;

	ibuf += 12;
	ilen -= 12;

	WORD block_count = *(WORD *)ibuf;

	ibuf += 2;

	WORD *block_length = (WORD *)ibuf;

	ibuf += 2*block_count;

	for (int i=0;i<block_count;i++)
	{
		WORD bul = *(WORD *)ibuf;

		if (unpack_block(ibuf + 2,block_length[i] - 2,obuf) != bul)
			return 0;

		obuf += bul;
		olen += bul;

		ibuf += block_length[i];
	}

	return olen;
}

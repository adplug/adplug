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

  dmo.cpp - TwinTeam loader by Riven the Mage <riven@ok.ru>
*/
/*
  NOTE: Panning is ignored.
*/

#include "dmo.h"

#define LOWORD(l) ((l) & 0xffff)
#define HIWORD(l) ((l) >> 16)
#define LOBYTE(w) ((w) & 0xff)
#define HIBYTE(w) ((w) >> 8)

/* -------- Public Methods -------------------------------- */

CPlayer *CdmoLoader::factory(Copl *newopl)
{
  CdmoLoader *p = new CdmoLoader(newopl);
  return p;
}

bool CdmoLoader::load(istream &f, const char *filename)
{
	int i,j;

	// check header
	dmo_unpacker *unpacker = new dmo_unpacker;
	
	unsigned char chkhdr[16];

	f.read(chkhdr,16);

	if (!unpacker->decrypt(chkhdr,16))
	{
		delete unpacker;
		return false;
	}

	// get file size
	f.seekg(0,ios::end);
	long packed_length = f.tellg();
	f.seekg(0);

	unsigned char *packed_module = new unsigned char [packed_length];

	// load file
	f.read((char *)packed_module,packed_length);

	// decrypt
	unpacker->decrypt(packed_module,packed_length);

	unsigned char *module = new unsigned char [0x2000 * (*(unsigned short *)&packed_module[12])];

	// unpack
	if (!unpacker->unpack(packed_module+12,module))
	{
		delete unpacker;
		delete packed_module;
		delete module;
		return false;
	}

	delete unpacker;
	delete packed_module;

	// "TwinTeam" - signed ?
	if (memcmp(module,"TwinTeam Module File""\x0D\x0A",22))
	{
		delete module;
		return false;
	}

	unsigned char *ibuf = module;

	// load header
	dmo_header *my_hdr = (dmo_header *)ibuf;

	memset(&header,0,sizeof(s3mheader));

	memcpy(header.name,my_hdr->title,28);

	header.ordnum  = my_hdr->numord;
	header.insnum  = my_hdr->numinst;
	header.patnum  = my_hdr->numpat;
	header.is      = my_hdr->speed;
	header.it      = my_hdr->tempo;

	memset(header.chanset,0xFF,32);

	for (i=0;i<9;i++)
		header.chanset[i] = 0x10 + i;

	ibuf += sizeof(dmo_header);

	// load order
	memcpy(orders,ibuf,256);

	orders[header.ordnum] = 0xFF;

	ibuf += 256;

	// load pattern lengths
	unsigned short *my_patlen = (unsigned short *)ibuf;

	ibuf += 200;

	// load instruments
	for (i=0;i<my_hdr->numinst;i++)
	{
		dmo_instrument *my_ins = (dmo_instrument *)ibuf;

		memset(&inst[i],0,sizeof(s3minst));

		inst[i].type   = my_ins->type;
		inst[i].d00    = my_ins->data[0];
		inst[i].d01    = my_ins->data[1];
		inst[i].d02    = my_ins->data[2];
		inst[i].d03    = my_ins->data[3];
		inst[i].d04    = my_ins->data[4];
		inst[i].d05    = my_ins->data[5];
		inst[i].d06    = my_ins->data[6];
		inst[i].d07    = my_ins->data[7];
		inst[i].d08    = my_ins->data[8];
		inst[i].d09    = my_ins->data[9];
		inst[i].d0a    = my_ins->data[10];
		inst[i].d0b    = my_ins->data[10];
		inst[i].volume = my_ins->vol;
		inst[i].dsk    = my_ins->dsk;
		inst[i].c2spd  = my_ins->c2spd;

		memcpy(inst[i].name,my_ins->name,28);

		ibuf += sizeof(dmo_instrument);
	}

	// load patterns
	for (i=0;i<my_hdr->numpat;i++)
	{
		unsigned char *cur_pat = ibuf;

		for (j=0;j<64;j++)
		{
			while (1)
			{
				unsigned char token = *cur_pat++;

				if (!token)
					break;

				unsigned char chan = token & 31;

				// note + instrument ?
				if (token & 32)
				{
					unsigned char bufbyte = *cur_pat++;

					pattern[i][j][chan].note = bufbyte & 15;
					pattern[i][j][chan].oct = bufbyte >> 4;
					pattern[i][j][chan].instrument = *cur_pat++;
				}

				// volume ?
				if (token & 64)
				{
					pattern[i][j][chan].volume = *cur_pat++;
				}

				// command ?
				if (token & 128)
				{
					pattern[i][j][chan].command = *cur_pat++;
					pattern[i][j][chan].info = *cur_pat++;
				}
			}
		}

		ibuf += my_patlen[i];
	}

	delete module;

	rewind(0);

	return true;
}

std::string CdmoLoader::gettype()
{
	return std::string("TwinTeam (packed S3M)");
}

std::string CdmoLoader::getauthor()
{
/*
  All available .DMO modules written by one composer. And because all .DMO
  stuff was lost due to hd crash (TwinTeam guys said this), there are
  never(?) be another.
*/
	return std::string("Benjamin GERARDIN");
}

/* -------- Private Methods ------------------------------- */

unsigned short CdmoLoader::dmo_unpacker::brand(unsigned short range)
{
	unsigned short ax,bx,cx,dx;

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

bool CdmoLoader::dmo_unpacker::decrypt(unsigned char *buf, long len)
{
	unsigned long seed = 0;
	int i;

	bseed = *(unsigned long *)&buf[0];

	for (i=0;i<(*(unsigned short *)&buf[4]+1);i++)
		seed += brand(0xffff);

	bseed = seed ^ *(unsigned long *)&buf[6];

	if ((*(unsigned short *)&buf[10]) != brand(0xffff))
		return false;

	for (i=0;i<(len-12);i++)
		buf[12+i] ^= brand(0x100);

	*(unsigned short *)&buf[len-2] = 0;

	return true;
}

short CdmoLoader::dmo_unpacker::unpack_block(unsigned char *ibuf, long ilen, unsigned char *obuf)
{
	unsigned char code,par1,par2;
	unsigned short ax,bx,cx;

	unsigned char *ipos = ibuf;
	unsigned char *opos = obuf;

	// LZ77 child
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
		  int i;

			par1 = *ipos++;

			ax = ((code & 0x3F) << 1) + (par1 >> 7) + 1;
			cx = ((par1 & 0x70) >> 4) + 3;
			bx = par1 & 0x0F;

			for(i=0;i<cx;i++)
				*opos++ = *(opos - ax);

			for (i=0;i<bx;i++)
				*opos++ = *ipos++;

			continue;
		}

		// 11xxxxxx xxxxxxxy yyyyzzzz: copy (Y + 4) from X; copy Z bytes
		if ((code >> 6) == 3)
		{
		  int i;

			par1 = *ipos++;
			par2 = *ipos++;

			bx = ((code & 0x3F) << 7) + (par1 >> 1);
			cx = ((par1 & 0x01) << 4) + (par2 >> 4) + 4;
			ax = par2 & 0x0F;

			for(i=0;i<cx;i++)
				*opos++ = *(opos - bx);

			for (i=0;i<ax;i++)
				*opos++ = *ipos++;

			continue;
		}
	}

	return opos - obuf;
}

long CdmoLoader::dmo_unpacker::unpack(unsigned char *ibuf, unsigned char *obuf)
{
	long olen = 0;

	unsigned short block_count = *(unsigned short *)ibuf;

	ibuf += 2;

	unsigned short *block_length = (unsigned short *)ibuf;

	ibuf += 2*block_count;

	for (int i=0;i<block_count;i++)
	{
		unsigned short bul = *(unsigned short *)ibuf;

		if (unpack_block(ibuf + 2,block_length[i] - 2,obuf) != bul)
			return 0;

		obuf += bul;
		olen += bul;

		ibuf += block_length[i];
	}

	return olen;
}

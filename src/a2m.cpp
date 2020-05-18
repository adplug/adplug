/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2007 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * a2m.cpp - A2M Loader by Simon Peter <dn.tlp@gmx.net>
 *
 * NOTES:
 * This loader detects and loads version 1, 4, 5 & 8 files.
 *
 * version 1-4 files:
 * Following commands are ignored: FF1 - FF9, FAx - FEx
 *
 * version 5-8 files:
 * Instrument panning is ignored. Flags byte is ignored.
 * Following commands are ignored: Gxy, Hxy, Kxy - &xy
 */

#include <cstring>
#include <cassert>
#include "a2m.h"

CPlayer *Ca2mLoader::factory(Copl *newopl)
{
  return new Ca2mLoader(newopl);
}

bool Ca2mLoader::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f = fp.open(filename); if(!f) return false;
  char id[10];
  int i,j,k,t;
  unsigned int l;
  unsigned char *org, *orgptr, flags = 0, numpats, version;
  unsigned long crc, alength;
  unsigned short len[9], *secdata, *secptr;
  const unsigned char convfx[16] = {0,1,2,23,24,3,5,4,6,9,17,13,11,19,7,14};
  const unsigned char convinf1[16] = {0,1,2,6,7,8,9,4,5,3,10,11,12,13,14,15};
  const unsigned char newconvfx[] = {0,1,2,3,4,5,6,23,24,21,10,11,17,13,7,19,
				     255,255,22,25,255,15,255,255,255,255,255,
				     255,255,255,255,255,255,255,255,14,255};

  // read header
  f->readString(id, 10); crc = f->readInt(4);
  version = f->readInt(1); numpats = f->readInt(1);

  // file validation section
  if (memcmp(id, "_A2module_", 10) ||
      (version != 1 && version != 5 && version != 4 && version != 8) ||
      numpats < 1 || numpats > 64) {
    fp.close(f);
    return false;
  }

  // load, depack & convert section
  nop = numpats; length = 128; restartpos = 0;
  if(version < 5) {
    for(i=0;i<5;i++) len[i] = f->readInt(2);
    t = 9;
  } else {	// version >= 5
    for(i=0;i<9;i++) len[i] = f->readInt(2);
    t = 18;
  }

  // block 0
  if(version == 1 || version == 5) {
    // Length of decoded data is known, see check below. But sixdepak()
    // has no output length parameter, so MAXBUF needs to be allocated.
    orgptr = org = new unsigned char [sixdepak::MAXBUF];
    secdata = new unsigned short [len[0] / 2];
    for(i=0;i<len[0]/2;i++) secdata[i] = f->readInt(2);
    // What if len[0] is odd: ignore, skip extra byte, or fail?
    l = sixdepak::decode(secdata, org, len[0]);
    delete [] secdata;
  } else {
    orgptr = org = new unsigned char [len[0]];
    for(l = 0; l < len[0]; l++) orgptr[l] = f->readInt(1);
  }
  if (l < 2*43 + 250*(33+13) + 128 + 2 + (version >= 5)) {
    // Block is too short; fail.
    delete [] org;
    fp.close(f);
    return false;
  }

  memcpy(songname,orgptr,43); orgptr += 43;
  memcpy(author,orgptr,43); orgptr += 43;
  memcpy(instname,orgptr,250*33); orgptr += 250*33;
  // If string length is invalid, just use the maximum. Should we fail instead?
  if (songname[0] > 42 || songname[0] < 0) songname[0] = 42;
  if (author[0] > 42 || author[0] < 0) author[0] = 42;

  for(i=0;i<250;i++) {	// instruments
    if (instname[i][0] > 32 || instname[i][0] < 0) instname[i][0] = 32;
    inst[i].data[0] = *(orgptr+i*13+10);
    inst[i].data[1] = *(orgptr+i*13);
    inst[i].data[2] = *(orgptr+i*13+1);
    inst[i].data[3] = *(orgptr+i*13+4);
    inst[i].data[4] = *(orgptr+i*13+5);
    inst[i].data[5] = *(orgptr+i*13+6);
    inst[i].data[6] = *(orgptr+i*13+7);
    inst[i].data[7] = *(orgptr+i*13+8);
    inst[i].data[8] = *(orgptr+i*13+9);
    inst[i].data[9] = *(orgptr+i*13+2);
    inst[i].data[10] = *(orgptr+i*13+3);

    if(version < 5)
      inst[i].misc = *(orgptr+i*13+11);
    else {	// version >= 5 -> OPL3 format
      int pan = *(orgptr+i*13+11);

      if(pan)
	inst[i].data[0] |= (pan & 3) << 4;	// set pan
      else
	inst[i].data[0] |= 48;			// enable both speakers
    }

    inst[i].slide = *(orgptr+i*13+12);
  }

  orgptr += 250*13;
  memcpy(order,orgptr,128); orgptr += 128;
  bpm = *orgptr; orgptr++;
  initspeed = *orgptr; orgptr++;
  if(version >= 5) flags = *orgptr;
  delete [] org;

  // blocks 1-4 or 1-8
  unsigned char ppb = version < 5 ? 16 : 8; // patterns per block
  unsigned char blocks = (numpats + ppb - 1) / ppb; // excluding block 0
  alength = len[1];
  for (i = 2; i <= blocks; i++)
    alength += len[i];

  if(version == 1 || version == 5) {
    org = new unsigned char [sixdepak::MAXBUF * blocks]; // again, we know better, but...
    secdata = new unsigned short [alength / 2];

    for (l = 0; l < alength / 2; l++)
      secdata[l] = f->readInt(2);

    orgptr = org; secptr = secdata;
    for (i = 1; i <= blocks; i++) {
      orgptr += sixdepak::decode(secptr, orgptr, len[i]);
      secptr += len[i] / 2;
    }
    delete [] secdata;
  } else {
    org = new unsigned char [alength];
    f->readString((char *)org, alength);
    orgptr = org + alength;
  }

  if (orgptr - org < numpats * 64 * t * 4) {
    delete [] org;
    fp.close(f);
    return false;
  }

  if(version < 5) {
    for(i=0;i<numpats;i++)
      for(j=0;j<64;j++)
	for(k=0;k<9;k++) {
	  struct Tracks	*track = &tracks[i * 9 + k][j];
	  unsigned char	*o = &org[i*64*t*4+j*t*4+k*4];

	  track->note = o[0] == 255 ? 127 : o[0];
	  track->inst = o[1];
	  track->command = o[2] < sizeof(convfx) ? convfx[o[2]] : 255;
	  track->param2 = o[3] & 0x0f;
	  if(track->command != 14)
	    track->param1 = o[3] >> 4;
	  else {
	    track->param1 = convinf1[o[3] >> 4];
	    if(track->param1 == 15 && !track->param2) {	// convert key-off
	      track->command = 8;
	      track->param1 = 0;
	      track->param2 = 0;
	    }
	  }
	  if(track->command == 14) {
	    switch(track->param1) {
	    case 2: // convert define waveform
	      track->command = 25;
	      track->param1 = track->param2;
	      track->param2 = 0xf;
	      break;
	    case 8: // convert volume slide up
	      track->command = 26;
	      track->param1 = track->param2;
	      track->param2 = 0;
	      break;
	    case 9: // convert volume slide down
	      track->command = 26;
	      track->param1 = 0;
	      break;
	    }
	  }
	}
  } else {	// version >= 5
    realloc_patterns(64, 64, 18);

    for(i=0;i<numpats;i++)
      for(j=0;j<18;j++)
	for(k=0;k<64;k++) {
	  struct Tracks	*track = &tracks[i * 18 + j][k];
	  unsigned char	*o = &org[i*64*t*4+j*64*4+k*4];

	  track->note = o[0] == 255 ? 127 : o[0];
	  track->inst = o[1];
	  track->command = o[2] < sizeof(newconvfx) ? newconvfx[o[2]] : 255;
	  track->param1 = o[3] >> 4;
	  track->param2 = o[3] & 0x0f;

	  // Convert '&' command
	  if(o[2] == 36)
	    switch(track->param1) {
	    case 0:	// pattern delay (frames)
	      track->command = 29;
	      track->param1 = 0;
	      // param2 already set correctly
	      break;

	    case 1:	// pattern delay (rows)
	      track->command = 14;
	      track->param1 = 8;
	      // param2 already set correctly
	      break;
	    }
	}
  }

  init_trackord();

  delete [] org;

  // Process flags
  if(version >= 5) {
    CmodPlayer::flags |= Opl3;				// All versions >= 5 are OPL3
    if(flags & 8) CmodPlayer::flags |= Tremolo;		// Tremolo depth
    if(flags & 16) CmodPlayer::flags |= Vibrato;	// Vibrato depth
  }

  // Note: crc value is not checked.

  fp.close(f);
  rewind(0);
  return true;
}

float Ca2mLoader::getrefresh()
{
	if(tempo != 18)
		return (float) (tempo);
	else
		return 18.2f;
}

/*** sixdepak methods *************************************/

unsigned short Ca2mLoader::sixdepak::bitvalue(unsigned short bit)
{
	assert(bit < copybits(COPYRANGES - 1));
	return 1 << bit;
}

unsigned short Ca2mLoader::sixdepak::copybits(unsigned short range)
{
	assert(range < COPYRANGES);
	return 2 * range + 4; // 4, 6, 8, 10, 12, 14
}

unsigned short Ca2mLoader::sixdepak::copymin(unsigned short range)
{
	assert(range < COPYRANGES);
	/*
	if (range > 0 )
		return bitvalue(copybits(range - 1)) + copymin(range - 1);
	else
		return 0;
	*/
	static const unsigned short table[COPYRANGES] = {
		0, 16, 80, 336, 1360, 5456
	};
	return table[range];
}

void Ca2mLoader::sixdepak::inittree()
{
	unsigned short i;

	for(i=2;i<=TWICEMAX;i++) {
		dad[i] = i / 2;
		freq[i] = 1;
	}

	for(i=1;i<=MAXCHAR;i++) {
		leftc[i] = 2 * i;
		rghtc[i] = 2 * i + 1;
	}
}

void Ca2mLoader::sixdepak::updatefreq(unsigned short a, unsigned short b)
{
	do {
		freq[dad[a]] = freq[a] + freq[b];
		a = dad[a];
		if(a != ROOT)
		{
			if(leftc[dad[a]] == a)
			{
				b = rghtc[dad[a]];
			}
			else
			{
				b = leftc[dad[a]];
			}
		}
	} while(a != ROOT);

	if(freq[ROOT] == MAXFREQ)
		for(a=1;a<=TWICEMAX;a++)
			freq[a] >>= 1;
}

void Ca2mLoader::sixdepak::updatemodel(unsigned short code)
{
	unsigned short a=code+SUCCMAX,b,c,code1,code2;

	freq[a]++;
	if(dad[a] != ROOT) {
		code1 = dad[a];
		if(leftc[code1] == a)
			updatefreq(a,rghtc[code1]);
		else
			updatefreq(a,leftc[code1]);

		do {
			code2 = dad[code1];
			if(leftc[code2] == code1)
				b = rghtc[code2];
			else
				b = leftc[code2];

			if(freq[a] > freq[b]) {
				if(leftc[code2] == code1)
					rghtc[code2] = a;
				else
					leftc[code2] = a;

				if(leftc[code1] == a) {
					leftc[code1] = b;
					c = rghtc[code1];
				} else {
					rghtc[code1] = b;
					c = leftc[code1];
				}

				dad[b] = code1;
				dad[a] = code2;
				updatefreq(b,c);
				a = b;
			}

			a = dad[a];
			code1 = dad[a];
		} while(code1 != ROOT);
	}
}

unsigned short Ca2mLoader::sixdepak::inputcode(unsigned short bits)
{
	unsigned short i,code=0;

	for(i=1;i<=bits;i++) {
		if(!ibitcount) {
			if(ibufcount == input_size)
				return 0;
			ibitbuffer = wdbuf[ibufcount];
			ibufcount++;
			ibitcount = 15;
		} else
			ibitcount--;

		if(ibitbuffer > 0x7fff)
			code |= bitvalue(i - 1);
		ibitbuffer <<= 1;
	}

	return code;
}

unsigned short Ca2mLoader::sixdepak::uncompress()
{
	unsigned short a=1;

	do {
		if(!ibitcount) {
			if(ibufcount == input_size)
				return TERMINATE;
			ibitbuffer = wdbuf[ibufcount];
			ibufcount++;
			ibitcount = 15;
		} else
			ibitcount--;

		if(ibitbuffer > 0x7fff)
			a = rghtc[a];
		else
			a = leftc[a];
		ibitbuffer <<= 1;
	} while(a <= MAXCHAR);

	a -= SUCCMAX;
	updatemodel(a);
	return a;
}

unsigned short Ca2mLoader::sixdepak::do_decode()
{
	unsigned short i,j,k,t,c,count=0,dist,len,index;

	ibitcount = 0; ibitbuffer = 0;
	obufcount = 0; ibufcount = 0;
	inittree();
	c = uncompress();

	while(c != TERMINATE) {
		if(c < 256) {
			obuf[obufcount] = (unsigned char)c;
			obufcount++;
			if(obufcount == MAXBUF)
				return MAXBUF;

			buf[count] = (unsigned char)c;
			count++;
			if(count == MAXSIZE)
				count = 0;
		} else {
			t = c - FIRSTCODE;
			index = t / CODESPERRANGE;
			len = t + MINCOPY - index * CODESPERRANGE;
			dist = inputcode(copybits(index)) + copymin(index) + len;

			j = count;
			k = count - dist;
			if(count < dist)
				k += MAXSIZE;

			for(i=0;i<=len-1;i++) {
				obuf[obufcount] = buf[k];
				obufcount++;
				if(obufcount == MAXBUF)
					return MAXBUF;

				buf[j] = buf[k];
				j++; k++;
				if(j == MAXSIZE) j = 0;
				if(k == MAXSIZE) k = 0;
			}

			count += len;
			if(count >= MAXSIZE)
				count -= MAXSIZE;
		}
		c = uncompress();
	}
	return obufcount;
}

Ca2mLoader::sixdepak::sixdepak(
	unsigned short *source, unsigned char *dest, unsigned short isize
) : wdbuf(source), obuf(dest), input_size(isize)
{
}

unsigned short Ca2mLoader::sixdepak::decode(
	unsigned short *source, unsigned char *dest, unsigned short size)
{
	if (size < 2 || size > MAXBUF - 4096) // why the max?
		return 0;

	sixdepak *decoder = new sixdepak(source, dest, size / 2);

	unsigned short out_size = decoder->do_decode();

	delete decoder;
	return out_size;
}

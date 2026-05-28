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
 * vgm.c - VGM Player by Stas'M <binarymaster@mail.ru>
 *
 * REFERENCES:
 * https://vgmrips.net/wiki/VGM_Specification
 * http://www.smspower.org/uploads/Music/vgmspec160.txt
 * http://www.smspower.org/uploads/Music/gd3spec100.txt
 */

#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "vgm.h"
#include "ungzip.h"

/*** Local helpers — VGM is always little-endian ***************************/

static inline uint8_t vgm_u8(const uint8_t *b, int p)
{
	return b[p];
}

static inline uint32_t vgm_u32(const uint8_t *b, int p)
{
	return (uint32_t)b[p]
	     | ((uint32_t)b[p + 1] <<  8)
	     | ((uint32_t)b[p + 2] << 16)
	     | ((uint32_t)b[p + 3] << 24);
}

/*** public methods *************************************/

CPlayer *CvgmPlayer::factory(Copl *newopl)
{
	return new CvgmPlayer(newopl);
}

/* Read one null-terminated UTF-16LE string from buf at position pos.
 * Returns the new position after the terminating null character. */
static int fillGD3Tag(const uint8_t *buf, long buflen, int pos, wchar_t *data)
{
	uint16_t chr;
	int cnt = 0;
	do
	{
		if (pos + 2 > (int)buflen)
		{
			data[cnt < 256 ? cnt : 255] = 0;
			break;
		}
		chr = (uint16_t)buf[pos] | ((uint16_t)buf[pos + 1] << 8);
		pos += 2;
		data[cnt < 256 ? cnt : 255] = (wchar_t)(cnt < 256 ? chr : 0);
		cnt++;
	} while (chr);
	return pos;
}

bool CvgmPlayer::load(const std::string &filename, const CFileProvider &fp)
{
	binistream *f = fp.open(filename);
	if (!f) return false;

	if (!fp.extension(filename, ".vgm") &&
		!fp.extension(filename, ".vgz"))
	{
		fp.close(f);
		return false;
	}

	long filesize = (long)fp.filesize(f);
	if (filesize < VGM_GZIP_MIN)
	{
		// File size is too small
		fp.close(f);
		return false;
	}

	// Load the entire file into a memory buffer
	uint8_t *buf = new uint8_t[filesize];
	f->readString((char *)buf, filesize);
	fp.close(f);

	// Decompress if this is a gzip stream (.vgz)
	if (buf[0] == 0x1F && buf[1] == 0x8B)
	{
		long outsize = ungzip(buf, filesize, NULL, 0);
		if (outsize <= 0)
		{
			delete[] buf;
			return false;
		}
		uint8_t *decomp = new uint8_t[outsize];
		long got = ungzip(buf, filesize, decomp, outsize);
		delete[] buf;
		if (got <= 0)
		{
			delete[] decomp;
			return false;
		}
		buf      = decomp;
		filesize = got;
	}

	// From here buf/filesize always hold plain (uncompressed) VGM data

	if (filesize < VGM_HEADER_MIN)
	{
		// File size is too small
		delete[] buf;
		return false;
	}
	if (memcmp(buf, VGM_HEADER_ID, 4) != 0)
	{
		// Header ID mismatch
		delete[] buf;
		return false;
	}

	uint32_t eof_field = vgm_u32(buf, OFFSET_EOF);
	if ((long)(eof_field + OFFSET_EOF) != filesize)
	{
		// EOF offset is incorrect
		delete[] buf;
		return false;
	}

	version = (int)vgm_u32(buf, 0x08);
	if (version < 0x151)
	{
		// Minimum supported VGM version is 1.51
		delete[] buf;
		return false;
	}

	samples  = (int)vgm_u32(buf, 0x18);
	loop_ofs = (int)vgm_u32(buf, OFFSET_LOOP);
	loop_smp = (int)vgm_u32(buf, 0x20);
	rate     = (int)vgm_u32(buf, 0x24);

	int data_ofs = (int)vgm_u32(buf, OFFSET_DATA);
	if (data_ofs < OFFSET_YM3812 - OFFSET_DATA + 4)
	{
		// VGM data overlays important header fields
		delete[] buf;
		return false;
	}

	clock = 0;
	if (data_ofs >= OFFSET_YMF262 - OFFSET_DATA + 4)
		clock = (int)vgm_u32(buf, OFFSET_YMF262);

	vgmOPL3 = (clock != 0);
	vgmOPL1  = false;
	vgmDual  = false;

	if (!vgmOPL3)
	{
		clock   = (int)vgm_u32(buf, OFFSET_YM3812);
		vgmDual = (clock & VGM_DUAL_BIT) > 0;
		clock  &= (VGM_DUAL_BIT - 1);
	}

	if (!vgmOPL3 && !clock)
	{
		if (data_ofs >= OFFSET_YM3526 - OFFSET_DATA + 4)
			clock = (int)(vgm_u32(buf, OFFSET_YM3526) & (VGM_DUAL_BIT - 1));
		vgmOPL1 = (clock != 0);
	}

	if (!clock)
	{
		// VGM clock is not set
		delete[] buf;
		return false;
	}

	loop_base = 0;
	if (data_ofs >= OFFSET_LOOPBASE - OFFSET_DATA + 1)
		loop_base = vgm_u8(buf, OFFSET_LOOPBASE);

	loop_mod = 0;
	if (data_ofs >= OFFSET_LOOPMOD - OFFSET_DATA + 1)
		loop_mod = vgm_u8(buf, OFFSET_LOOPMOD);

	data_sz = 0;
	int gd3_ofs = (int)vgm_u32(buf, OFFSET_GD3);
	if (gd3_ofs)
	{
		// Process GD3 tag
		int gd3_pos = OFFSET_GD3 + gd3_ofs;
		if (gd3_pos + 12 <= filesize &&
			memcmp(buf + gd3_pos, GD3_HEADER_ID, 4) == 0)
		{
			gd3_pos += 12; // skip: ID(4) + version(4) + length(4)
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.title_en);
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.title_jp);
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.game_en);
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.game_jp);
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.system_en);
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.system_jp);
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.author_en);
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.author_jp);
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.date);
			gd3_pos = fillGD3Tag(buf, filesize, gd3_pos, GD3.ripper);
			(void)    fillGD3Tag(buf, filesize, gd3_pos, GD3.notes);
		}
	}
	else
	{
		gd3_ofs = (int)vgm_u32(buf, OFFSET_EOF);
	}

	int data_start = OFFSET_DATA + data_ofs;
	data_sz = gd3_ofs - data_ofs;
	if (data_sz <= 0 || data_start + data_sz > filesize)
	{
		delete[] buf;
		return false;
	}

	vgmData = new uint8_t[data_sz];
	memcpy(vgmData, buf + data_start, data_sz);
	delete[] buf;

	loop_ofs -= data_ofs + (OFFSET_DATA - OFFSET_LOOP);
	rewind(0);
	return true;
}

bool CvgmPlayer::update()
{
	uint8_t reg, val;
	wait = 0;

	do
	{
		if (pos >= data_sz)
		{
			songend = true;
			break;
		}
		uint8_t cmd = vgmData[pos++];
		switch (cmd)
		{
		case CMD_OPL1:
		case CMD_OPL2:
		case CMD_OPL3_PORT0:
			reg = vgmData[pos++];
			val = vgmData[pos++];
			if ((vgmOPL1 && cmd == CMD_OPL1) || (!vgmOPL3 && cmd == CMD_OPL2) || (vgmOPL3 && cmd == CMD_OPL3_PORT0))
			{
				if (opl->getchip() != 0)
					opl->setchip(0);
				opl->write(reg, val);
			}
			break;
		case CMD_OPL2_2ND:
		case CMD_OPL3_PORT1:
			reg = vgmData[pos++];
			val = vgmData[pos++];
			if ((vgmDual && cmd == CMD_OPL2_2ND) || (vgmOPL3 && cmd == CMD_OPL3_PORT1))
			{
				if (opl->getchip() != 1)
					opl->setchip(1);
				opl->write(reg, val);
			}
			break;
		case CMD_WAIT:
			wait  = vgmData[pos++];
			wait |= vgmData[pos++] << 8;
			break;
		case CMD_WAIT_735:
			wait = 735;
			break;
		case CMD_WAIT_882:
			wait = 882;
			break;
		case CMD_DATA_END:
			pos = data_sz;
			break;
		default:
			if (cmd >= CMD_WAIT_N && cmd <= CMD_WAIT_N + 0xF)
			{
				wait = (cmd & 0xF) + 1;
			}
		}
		if (wait && wait < 40)
			wait = 0; // skip too short pauses
		if (!songend)
			songend = pos >= data_sz;
		if (pos >= data_sz && loop_ofs >= 0)
			pos = loop_ofs;
	} while (!wait);
	return !songend;
}

void CvgmPlayer::rewind(int subsong)
{
	pos = 0; songend = false; wait = 0;
	opl->init();
}

float CvgmPlayer::getrefresh()
{
	return VGM_FREQUENCY / (wait > 0 ? wait : VGM_FREQUENCY / 1000);
}

std::string CvgmPlayer::gettype()
{
	char chip[10];
	memset(chip, 0, 10);
	if (vgmOPL3)
		strcpy(chip, "OPL3");
	else if (vgmDual)
		strcpy(chip, "Dual OPL2");
	else if (vgmOPL1)
		strcpy(chip, "OPL1");
	else
		strcpy(chip, "OPL2");
	char tmpstr[40];
	uint8_t major = (version >> 8) & 0xFF;
	uint8_t minor = version & 0xFF;
	snprintf(tmpstr, sizeof(tmpstr), "Video Game Music %x.%x (%s)", major, minor, chip);
	return std::string(tmpstr);
}

std::string CvgmPlayer::gettitle()
{
	char str[256];
	str[0] = 0;
	if (GD3.title_en[0])
	{
		wcstombs(str, GD3.title_en, 256);
	}
	else if (GD3.title_jp[0])
	{
		wcstombs(str, GD3.title_jp, 256);
	}
	return std::string(str);
}

std::string CvgmPlayer::getauthor()
{
	char str[256];
	str[0] = 0;
	if (GD3.author_en[0])
	{
		wcstombs(str, GD3.author_en, 256);
	}
	else if (GD3.author_jp[0])
	{
		wcstombs(str, GD3.author_jp, 256);
	}
	return std::string(str);
}

std::string CvgmPlayer::getdesc()
{
	char game[256]; game[0] = 0;
	char system[256]; system[0] = 0;
	char date[256]; date[0] = 0;
	char notes[256]; notes[0] = 0;

	if (GD3.game_en[0])
	{
		wcstombs(game, GD3.game_en, 256);
	}
	else if (GD3.game_jp[0])
	{
		wcstombs(game, GD3.game_jp, 256);
	}
	if (GD3.system_en[0])
	{
		wcstombs(system, GD3.system_en, 256);
	}
	else if (GD3.system_jp[0])
	{
		wcstombs(system, GD3.system_jp, 256);
	}
	if (GD3.date[0])
		wcstombs(date, GD3.date, 256);
	if (GD3.notes[0])
		wcstombs(notes, GD3.notes, 256);
	char str_sys[256]; str_sys[0] = 0;
	if (system[0] && date[0] && strlen(system) <= 251)
	{
		snprintf(str_sys, sizeof(str_sys), "%.251s / %.*s", system, 252 - (int)strlen(system), date);
	}
	else if (system[0])
	{
		strcpy(str_sys, system);
	}
	else if (date[0])
	{
		strcpy(str_sys, date);
	}
	char str_game[256]; str_game[0] = 0;
	char str_desc[256]; str_desc[0] = 0;
	if (game[0])
	{
		if (str_sys[0] && strlen(game) <= 251)
		{
			snprintf(str_game, sizeof(str_game), "%.251s (%.*s)", game, 252 - (int)strlen(game), str_sys);
		}
		else
		{
			strcpy(str_game, game);
		}
	}
	else if (str_sys[0])
	{
		strcpy(str_game, str_sys);
	}
	if (notes[0] && strlen(str_game) <= 250)
	{
		snprintf(str_desc, sizeof(str_desc), "%.250s\r\n\r\n%.*s", str_game, 251 - (int)strlen(str_game), notes);
	}
	else
	{
		strcpy(str_desc, str_game);
	}
	return std::string(str_desc);
}

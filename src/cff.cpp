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
  cff.cpp - BoomTracker loader by Riven the Mage <riven@ok.ru>
*/

#include "cff.h"

/* -------- Public Methods -------------------------------- */

CPlayer *CcffLoader::factory(Copl *newopl)
{
  CcffLoader *p = new CcffLoader(newopl);
  return p;
}

bool CcffLoader::load(istream &f, const char *filename)
{
	f.read((char *)&header,sizeof(header));

	// '<CUD-FM-File>' - signed ?
	if (memcmp(header.id,"<CUD-FM-File>""\x1A\xDE\xE0",16))
		return false;

	unsigned char *module = new unsigned char [header.size];

	// packed ?
	if (header.packed)
	{
		unsigned char *packed_module = new unsigned char [header.size];

		f.read((char *)packed_module,header.size);

		if (!unpack(packed_module,module))
		{
			delete packed_module;
			delete module;
			return false;
		}

		delete packed_module;
	}
	else
	{
		f.read((char *)module,header.size);
	}










	delete module;

	rewind(0);

	return true;	
}

std::string CcffLoader::gettype()
{
#ifndef DETAIL_INFO
	std::string xstr = "BoomTracker 4";
#else
	std::string xstr = "BoomTracker 4.0+ (version 1)";
#endif

	if (header.packed)
		xstr += ", packed";

	return std::string(xstr);
}

/* -------- Private Methods ------------------------------- */

#ifdef _WIN32
	#pragma warning(disable:4244)
	#pragma warning(disable:4018)
#endif

/*
  Lempel-Ziv-Tyr ;-)
*/
unsigned int CcffLoader::cff_unpacker::unpack(unsigned char *ibuf, unsigned char *obuf)
{
	if (memcmp(ibuf,"YsComp""\x07""CUD1997""\x1A\x04",16))
		return 0;

	input = ibuf;
	output = obuf;

	heap = (unsigned char *)malloc(0x10000);
	dictionary = (unsigned char **)malloc(sizeof(unsigned char *)*0x8000);

	memset(heap,0,0x10000);
	memset(dictionary,0,0x8000);

	cleanup();
	startup();

	// LZW
	while (1)
	{
		new_code = get_code();

		// 0x00: end of data
		if (new_code == 0)
			break;

		// 0x01: end of block
		if (new_code == 1)
		{
			cleanup();
			startup();

			continue;
		}

		// 0x02: expand code length
		if (new_code == 2)
		{
			code_length++;

			continue;
		}

		// 0x03: RLE
		if (new_code == 3)
		{
			unsigned char old_code_length = code_length;

			code_length = 2;

			unsigned char symbol_length = get_code() + 1;

			code_length = 4 << get_code();

			unsigned long repeat_counter = get_code();

			for (int i=0;i<repeat_counter*symbol_length;i++)
				output[output_length++] = output[output_length - symbol_length];

			code_length = old_code_length;

			startup();

			continue;
		}

		if (new_code >= (0x104 + dictionary_length))
		{
			// dictionary <- old.code.string + old.code.char
			the_string[++the_string[0]] = the_string[1];
		}
		else
		{
			// dictionary <- old.code.string + new.code.char
			unsigned char temp_string[256];

			translate_code(new_code,temp_string);

			the_string[++the_string[0]] = temp_string[1];
		}

		expand_dictionary(the_string);

		// output <- new.code.string
		translate_code(new_code,the_string);

		for (int i=0;i<the_string[0];i++)
			output[output_length++] = the_string[i+1];

		old_code = new_code;
	}

	free(heap);
	free(dictionary);

	if (memcmp(output + 0x5E1,"CUD-FM-File - SEND A POSTCARD -",31))
		return 0;

	return output_length;
}

unsigned long CcffLoader::cff_unpacker::get_code()
{
	unsigned long code;

	while (bits_left <= 24)
	{
		bits_buffer |= ((*input++) << bits_left);
		bits_left += 8;
	}

	code = bits_buffer & ((1 << code_length) - 1);

	bits_buffer >>= code_length;
	bits_left -= code_length;

	return code;
}

void CcffLoader::cff_unpacker::translate_code(unsigned long code, unsigned char *string)
{
	unsigned char translated_string[256];

	if (code >= 0x104)
	{
		memcpy(translated_string,dictionary[code - 0x104],(*(dictionary[code - 0x104])) + 1);
	}
	else
	{
		translated_string[0] = 1;
		translated_string[1] = (code - 4) & 0xFF;
	}

	memcpy(string,translated_string,256);
}

void CcffLoader::cff_unpacker::cleanup()
{
	code_length = 9;

	bits_buffer = 0;
	bits_left = 0;

	heap_length = 0;
	dictionary_length = 0;
}

void CcffLoader::cff_unpacker::startup()
{
	old_code = get_code();

	translate_code(old_code,the_string);

	for (int i=0;i<the_string[0];i++)
		output[output_length++] = the_string[i+1];
}

void CcffLoader::cff_unpacker::expand_dictionary(unsigned char *string)
{
	memcpy(&heap[heap_length],string,string[0] + 1);

	dictionary[dictionary_length] = &heap[heap_length];

	dictionary_length++;

	heap_length += (string[0] + 1);
}

#ifdef _WIN32
	#pragma warning(default:4244)
	#pragma warning(default:4018)
#endif

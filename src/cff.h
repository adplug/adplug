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

#include "protrack.h"

class CcffLoader: public CmodPlayer
{
	public:

		static CPlayer *factory(Copl *newopl);

		CcffLoader(Copl *newopl) : CmodPlayer(newopl) { };

		bool            load(istream &f, const char *filename);

		std::string     gettype();

	private:

		class cff_unpacker
		{
			public:

				unsigned int unpack(unsigned char *ibuf, unsigned char *obuf);

			private:

				unsigned long get_code();
				void translate_code(unsigned long code, unsigned char *string);

				void cleanup();
				void startup();

				void expand_dictionary(unsigned char *string);

				unsigned char *input;
				unsigned char *output;

				unsigned int output_length;

				unsigned char code_length;

				unsigned long bits_buffer;
				unsigned int bits_left;

				unsigned char *heap;
				unsigned char **dictionary;

				unsigned int heap_length;
				unsigned int dictionary_length;

				unsigned long old_code,new_code;

				unsigned char the_string[256];
		};

        	struct cff_header
		{
			char		id[16];
			char		version;
			unsigned short	size;
			char		packed;
			char		reserved[12];
		} header;
};

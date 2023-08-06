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
 * crctest.cpp - Test AdPlug CRC calculation, by Stas'M <binarymaster@mail.ru>
 */

#include <stdlib.h>
#include <stdio.h>
#include <binfile.h>
#include <sys/stat.h>

#include "../src/fprovide.h"
#include "../src/database.h"

#ifdef MSDOS
#	define DIR_DELIM	"\\"
#else
#	define DIR_DELIM	"/"
#endif

#define SUBDIR  "testmus"

/***** Local variables *****/

// String holding the relative path to the source directory
static const char *testdir;

struct crcEntry
{
	const char *filename;
	unsigned short crc16;
	unsigned long  crc32;
};

static const crcEntry testlist[] = {
	{"2001.MKJ", 0x5138, 0xFA44E548},
	{"ADAGIO.DFM", 0x0021, 0xFC217809},
	{"adlibsp.s3m", 0x40A8, 0xC3A4D5BF},
	{NULL, 0, 0}
};

/***** Main program *****/

static bool dir_exists(std::string path)
{
	struct stat info;

	int rc = stat(path.c_str(), &info);

	if (rc != 0)
		return false;

	return (info.st_mode & S_IFDIR);
}

int main(int argc, char *argv[])
{
	bool retval = true;
	bool use_subdir;
	const CFileProvider &fp = CProvider_Filesystem();

	// Set path to source directory
	testdir = getenv("testdir");
	if (!testdir)
		testdir = ".";

	use_subdir = dir_exists(std::string(testdir) + DIR_DELIM SUBDIR);

	for (int i = 0; testlist[i].filename != NULL; i++)
	{
		std::string path = std::string(testdir) + DIR_DELIM;
		if (use_subdir)
			path += SUBDIR DIR_DELIM;
		path += testlist[i].filename;

		binistream *f = fp.open(path);
		if (!f)
		{
			std::cerr << "Error opening for reading: " << testlist[i].filename << "\n";
			retval = false;
			continue;
		}

		f->seek(0);
		CAdPlugDatabase::CKey key(*f);
		fp.close(f);
		std::cout << "Checking CRC16: " << testlist[i].filename;
		if (testlist[i].crc16 != key.crc16)
		{
			std::cout << " [FAIL: " << std::hex << key.crc16 << "]\n";
			retval = false;
		}
		else
			std::cout << " [OK]\n";
		std::cout << "Checking CRC32: " << testlist[i].filename;
		if (testlist[i].crc32 != key.crc32)
		{
			std::cout << " [FAIL: " << std::hex << key.crc32 << "]\n";
			retval = false;
		}
		else
			std::cout << " [OK]\n";
	}

	return retval ? EXIT_SUCCESS : EXIT_FAILURE;
}

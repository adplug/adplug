/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * stresstest.cpp - Test robustness when dealing with possibly malformed
 * and hostile input (e.g. fuzzing samples). Should be used with a
 * memory error detector (e.g. asan, valgrind) or errors may remain
 * undetected.
 */

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <string>
#include <signal.h>
#ifndef WEXITSTATUS
#include <sys/wait.h>
#endif

#include "../src/adplug.h"
#include "../src/opl.h"

#ifdef MSDOS
#	define DIR_DELIM	"\\"
#else
#	define DIR_DELIM	"/"
#endif

/***** Local variables *****/

static const char *const filelist[] = {
	/* Example */
	"fdance03.dmo",	// TwinTrack sample (skipped in playertest)
	/* Regression tests */
	"i-85.bmf",	// Issue #85 (CVE-2019-14690): "Heap-based buffer
			//    overflow in CxadbmfPlayer::__bmf_convert_stream()"
	"i-86.dtm",	// Issue #86 (CVE-2019-14691): "Heap-based buffer
			//            overflow in CdtmLoader::load()"
	"i-87.mkj",	// Issue #87 (CVE-2019-14692): "Heap-based buffer
			//            overflow in CmkjPlayer::load()"
	"i-88_1.a2m",	// Issue #88 (CVE-2019-14732): "Multiple heap-based
	"i-88_2.a2m",	//            buffer overflows in Ca2mLoader::load()"
	"i-89_1.rad",	// Issue #89 (CVE-2019-14733): "Multiple heap-based
	"i-89_2.rad",	//            buffer overflows in CradLoader::load()"
	"i-90_1.mtk",	// Issue #90 (CVE-2019-14734): "Multiple heap-based
	"i-90_2.mtk",	//            buffer overflows in CmtkLoader::load()"
	"i-91.m",	// Issue #91 (CVE-2019-15151): "Double free in
			//            Cu6mPlayer::~Cu6mPlayer()"
	"i-92_1.m",	// Issee #92: "Multiple OOB reads in
	"i-92_2.m",	//            Cu6mPlayer::get_next_codeword()"
	"i-92_3.m", "i-92_4.m",
	"i-93_01.bmf",	// Issue #93: "Multiple OOB reads in
	"i-93_02.bmf",	//            CxadbmfPlayer::xadplayer_load()"
	"i-93_03.xad", "i-93_04.bmf", "i-93_05.xad", "i-93_06.xad",
	"i-93_07.bmf", "i-93_08.xad", "i-93_09.xad", "i-93_10.xad",
	"i-93_11.xad", "i-93_12.xad", "i-93_13.xad", "i-93_14.xad",
	"i-93_15.bmf", "i-93_16.xad", "i-93_17.xad", "i-93_18.xad",
	"i-94.rix",	// Issue #94: "OOB read in CrixPlayer::data_initial()"
	"i-95_1.ctm",	// Issue #95: "Floating-point exception abort (DoS) in
	"i-95_2.ctm",	//            CmidPlayer::rewind()"
	"i-96_1.rol",	// Issue #96: "Reachable memory allocation abort (DoS)
	"i-96_2.rol",	//            in CrolPlayer::load_tempo_events()"
	"i-96_3.rol",
	"i-98.sng",	// Issue #98: "Floating-point exception abort (DoS) in
			//            CfmcLoader::load()"
	"i-99.dro",	// Issue #99: "Heap-based OOB write (memory corruption)
			//            in CdroPlayer::load()"
	"i-100_01.xad",	// Issue #100: "Multiple excessive memory allocations
	"i-100_02.xad",	//            in loaders"
	"i-100_03.dro", "i-100_04.dro", "i-100_05.raw", "i-100_06.raw",
	"i-100_07.dro", "i-100_08.dro", "i-100_09.adlib", "i-100_10.mkj",
	"i-100_11.mkj", "i-100_12.cmf", "i-100_13.cmf",
	"i-101_1.d00",	// Issue #101: "Multiple null pointer dereferences (DoS)
	"i-101_2.d00",	//            in Cd00Player"
};

static const int filecount = sizeof(filelist) / sizeof(filelist[0]);

/***** SilentTestopl *****/

// Like CSilentopl, but prints warnings when data is out of range
class SilentTestopl : public Copl {
public:

  void write(int reg, int val)
  {
    if (reg > 255 || val > 255 || reg < 0 || val < 0)
      std::cerr << "Warning: The player is writing data out of range! (reg = "
		<< std::hex << reg << ", val = " << val << ")\n";
  }

  void init() {}
};

/***** Local functions *****/

bool run_test(int argc, const char *const argv[])
{
	// For now, a single argument is expected: a file name.
	// Future test cases may provide additional arguments.
	if (argc != 1)
		std::cerr << "Warning: unsupported # of arguments (got "
			<< argc << ", expected 1)\n";

	SilentTestopl opl;
	// Load test file
	std::cout << "loading " << argv[0] << " ...";
	std::cout.flush();
	CPlayer *p = CAdPlug::factory(argv[0], &opl);
	if (p) {
		std::cout << " done\n";
	} else {
		std::cout << " failed\n";
		return false;
	}

	// Output file information
	std::cout << " - type:        " << p->gettype()   << std::endl;
	std::cout << " - title:       " << p->gettitle()  << std::endl;
	std::cout << " - author:      " << p->getauthor() << std::endl;
	std::cout << " - description: " << p->getdesc()   << std::endl;

	// Process the file
	while(p->update()) {
		float refresh = p->getrefresh();
		// With a CEmuopl or similar, SAMPLERATE/refresh sound
		// samples are available and can be read with:
		//opl.update(buf, n);
	}

	delete p;
	return true;
}

static bool test_wrapper(const std::string &cmdprefix, const char *file)
{
	std::string cmd = cmdprefix + file;

	// A test failure means unsuccessful process termination. In order
	// to catch such a failure, create a child process for each test.
	std::cout.flush();
	int status = system(cmd.c_str());

	std::cout << "Testing: " << file;
	if (WIFSIGNALED(status)) {
		std::cout << " - [FAIL] (killed by signal "
			 << WTERMSIG(status) << ")\n";
		if (WTERMSIG(status) == SIGINT || WTERMSIG(status) == SIGQUIT) {
			std::cout << "Quit.\n";
			exit(EXIT_FAILURE);
		}
	} else if (WEXITSTATUS(status) != EXIT_SUCCESS) {
		std::cout << " - [FAIL] (exit " << WEXITSTATUS(status) << ")\n";
	} else {
		std::cout << " - [OK]\n";
		return true;
	}
	return false;
}

/***** Main program *****/

int main(int argc, char *argv[])
{
	if (argc > 1 && !strcmp(argv[1], "+")) {
		// This is a child process for a single test; run it!
		run_test(argc - 2, argv + 2);
		// Succeed if it didn't crash or exit prematurely
		return EXIT_SUCCESS;
	}

	// Prepare a command prefix to recursively invoke ourselves
	std::string cmd;
	// Optional wrapper executable to run test cases
	const char *wrapper = getenv("stresstest_wrapper");
	if (wrapper) {
		cmd = wrapper;
		cmd += ' ';
	}
	cmd += argv[0]; // Re-exec ourselves
	cmd += " + ";

	// Set path to source directory
	const char *testdir = getenv("testdir");
	if (testdir) {
		cmd += testdir;
		int l = strlen(DIR_DELIM);
		if (cmd.compare(cmd.size() - l, l, DIR_DELIM) != 0)
			cmd += DIR_DELIM;
	}

	bool fail = false;
	if (argc < 2) {
		// No files, so run all tests from filelist.
		for (int i = 0; i < filecount; i++)
			fail |= !test_wrapper(cmd, filelist[i]);
	} else {
		// Test the file(s) on the command line
		for(int i = 1; i < argc; i++)
			fail |= !test_wrapper(cmd, argv[i]);
	}

	return fail ? EXIT_FAILURE : EXIT_SUCCESS;
}

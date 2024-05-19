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
#include <unistd.h>
#ifndef WEXITSTATUS
#include <sys/wait.h>
#endif
#include <sys/stat.h>

#include "../src/adplug.h"
#include "../src/opl.h"

#ifdef MSDOS
#	define DIR_DELIM	"\\"
#else
#	define DIR_DELIM	"/"
#endif

#define SUBDIR  "fuzzing"

/***** Local variables *****/

static const char *const filelist[] = {
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
	"i-102_001.dfm",// Issue #102: "Wild write (memory corruption) at user-
	"i-102_002.dfm",//            controlled address in CdfmLoader::load()"
	"i-102_003.dfm", "i-102_004.dfm", "i-102_005.dfm", "i-102_006.dfm",
	"i-102_007.dfm", "i-102_008.dfm", "i-102_009.dfm", "i-102_010.dfm",
	"i-102_011.dfm", "i-102_012.dfm", "i-102_013.dfm", "i-102_014.dfm",
	"i-102_015.dfm", "i-102_016.dfm", "i-102_017.dfm", "i-102_018.dfm",
	"i-102_019.dfm", "i-102_020.dfm", "i-102_021.dfm", "i-102_022.dfm",
	"i-102_023.dfm", "i-102_024.dfm", "i-102_025.dfm", "i-102_026.dfm",
	"i-102_027.dfm", "i-102_028.dfm", "i-102_029.dfm", "i-102_030.dfm",
	"i-102_031.dfm", "i-102_032.dfm", "i-102_033.dfm", "i-102_034.dfm",
	"i-102_035.dfm", "i-102_036.dfm", "i-102_037.dfm", "i-102_038.dfm",
	"i-102_039.dfm", "i-102_040.dfm", "i-102_041.dfm", "i-102_042.dfm",
	"i-102_043.dfm", "i-102_044.dfm", "i-102_045.dfm", "i-102_046.dfm",
	"i-102_047.dfm", "i-102_048.dfm", "i-102_049.dfm", "i-102_050.dfm",
	"i-102_051.dfm", "i-102_052.dfm", "i-102_053.dfm", "i-102_054.dfm",
	"i-102_055.dfm", "i-102_056.dfm", "i-102_057.dfm", "i-102_058.dfm",
	"i-102_059.dfm", "i-102_060.dfm", "i-102_061.dfm", "i-102_062.dfm",
	"i-102_063.dfm", "i-102_064.dfm", "i-102_065.dfm", "i-102_066.dfm",
	"i-102_067.dfm", "i-102_068.dfm", "i-102_069.dfm", "i-102_070.dfm",
	"i-102_071.dfm", "i-102_072.dfm", "i-102_073.dfm", "i-102_074.dfm",
	"i-102_075.dfm", "i-102_076.dfm", "i-102_077.dfm", "i-102_078.dfm",
	"i-102_079.dfm", "i-102_080.dfm", "i-102_081.dfm", "i-102_082.dfm",
	"i-102_083.dfm", "i-102_084.dfm", "i-102_085.dfm", "i-102_086.dfm",
	"i-102_087.dfm", "i-102_088.dfm", "i-102_089.dfm", "i-102_090.dfm",
	"i-102_091.dfm", "i-102_092.dfm", "i-102_093.dfm", "i-102_094.dfm",
	"i-102_095.dfm", "i-102_096.dfm", "i-102_097.dfm", "i-102_098.dfm",
	"i-102_099.dfm", "i-102_100.dfm", "i-102_101.dfm", "i-102_102.dfm",
	"i-102_103.dfm", "i-102_104.dfm", "i-102_105.dfm", "i-102_106.dfm",
	"i-102_107.dfm", "i-102_108.dfm", "i-102_109.dfm", "i-102_110.dfm",
	"i-102_111.dfm", "i-102_112.dfm", "i-102_113.dfm", "i-102_114.dfm",
	"i-102_115.dfm", "i-102_116.dfm", "i-102_117.dfm", "i-102_118.dfm",
	"i-102_119.dfm", "i-102_120.dfm", "i-102_121.dfm", "i-102_122.dfm",
	"i-102_123.dfm", "i-102_124.dfm", "i-102_125.dfm", "i-102_126.dfm",
	"i-102_127.dfm", "i-102_128.dfm", "i-102_129.dfm", "i-102_130.dfm",
	"i-102_131.dfm", "i-102_132.dfm", "i-102_133.dfm", "i-102_134.dfm",
	"i-102_135.dfm", "i-102_136.dfm", "i-102_137.dfm", "i-102_138.dfm",
	"i-102_139.dfm", "i-102_140.dfm", "i-102_141.dfm", "i-102_142.dfm",
	"i-102_143.dfm", "i-102_144.dfm", "i-102_145.dfm", "i-102_146.dfm",
	"i-102_147.dfm", "i-102_148.dfm", "i-102_149.dfm", "i-102_150.dfm",
	"i-102_151.dfm",
	"i-110_001.dtm",// Issue #110: "Multiple additional errors, mostly
	"i-110_002.dtm",//             OOB reads, found by AFL++"
	"i-110_003.dtm", "i-110_004.dtm", "i-110_005.a2m", "i-110_006.a2m",
	"i-110_007.a2m", "i-110_008.a2m", "i-110_009.a2m", "i-110_010.a2m",
	"i-110_011.a2m", "i-110_012.cff", "i-110_013.cff", "i-110_014.cff",
	"i-110_015.cff", "i-110_016.cff", "i-110_017.cff", "i-110_018.cff",
	"i-110_019.cff", "i-110_020.cff", "i-110_021.cff", "i-110_022.cff",
	"i-110_023.cff", "i-110_024.cff", "i-110_025.amd", "i-110_026.amd",
	"i-110_027.amd", "i-110_028.amd", "i-110_029.amd", "i-110_030.xad",
	"i-110_031.dtm", "i-110_032.dtm", "i-110_033.amd", "i-110_034.xad",
	"i-110_035.xad", "i-110_036.xad", "i-110_037.xad", "i-110_038.a2m",
	"i-110_039.dtm", "i-110_040.dtm", "i-110_041.a2m", "i-110_042.a2m",
	"i-110_043.amd", "i-110_044.a2m", "i-110_045.a2m", "i-110_046.amd",
	"i-110_047.sng", "i-110_048.cff", "i-110_049.sa2", "i-110_050.sa2",
	"i-110_051.amd", "i-110_052.dtm", "i-110_053.dtm", "i-110_054.dtm",
	"i-110_055.dtm", "i-110_056.amd", "i-110_057.mtk", "i-110_058.xad",
	"i-110_059.dtm", "i-110_060.dtm", "i-110_061.dtm", "i-110_062.amd",
	"i-110_063.dtm", "i-110_064.dtm", "i-110_065.amd", "i-110_066.sng",
	"i-110_067.sat", "i-110_068.amd", "i-110_069.amd", "i-110_070.xad",
	"i-110_071.amd", "i-110_072.amd", "i-110_073.a2m", "i-110_074.a2m",
	"i-110_075.a2m", "i-110_076.a2m", "i-110_077.a2m", "i-110_078.a2m",
	"i-110_079.sng", "i-110_080.sng", "i-110_081.sa2", "i-110_082.sa2",
	"i-110_083.amd", "i-110_084.amd", "i-110_085.amd", "i-110_086.amd",
	"i-110_087.amd", "i-110_088.amd", "i-110_089.xad", "i-110_090.xad",
	"i-110_091.a2m", "i-110_092.cff", "i-110_093.cff", "i-110_094.mtk",
	"i-110_095.xad", "i-110_096.xad", "i-110_097.cff", "i-110_098.amd",
	"i-110_099.amd", "i-110_100.dtm", "i-110_101.cff", "i-110_102.cff",
	"i-110_103.cff", "i-110_104.amd", "i-110_105.cff", "i-110_106.sat",
	"i-110_107.a2m", "i-110_108.a2m", "i-110_109.amd", "i-110_110.dtm",
	"i-110_111.dtm", "i-110_112.dtm", "i-110_113.dtm", "i-110_114.dtm",
	"i-110_115.dtm", "i-110_116.dtm", "i-110_117.dtm", "i-110_118.dtm",
	"i-110_119.dtm", "i-110_120.dtm", "i-110_121.dtm", "i-110_122.dtm",
	"i-110_123.xad", "i-110_124.a2m", "i-110_125.dtm", "i-110_126.a2m",
	"i-110_127.amd", "i-110_128.dtm", "i-110_129.a2m", "i-110_130.amd",
	"i-110_131.amd", "i-110_132.a2m", "i-110_133.dtm", "i-110_134.sat",
	"i-110_135.amd", "i-110_136.amd", "i-110_137.a2m", "i-110_138.a2m",
	"i-110_139.dtm", "i-110_140.sat", "i-110_141.amd", "i-110_142.sat",
	"i-110_143.dtm", "i-110_144.xad", "i-110_145.amd", "i-110_146.xad",
	"i-110_147.amd", "i-110_148.sa2", "i-110_149.sa2", "i-110_150.sa2",
	"i-110_151.amd", "i-110_152.amd", "i-110_153.amd", "i-110_154.amd",
	"i-110_155.amd", "i-110_156.amd", "i-110_157.a2m", "i-110_158.amd",
	"i-110_159.a2m", "i-110_160.amd", "i-110_161.xad", "i-110_162.sa2",
	"i-110_163.sa2", "i-110_164.dtm", "i-110_165.dtm", "i-110_166.dtm",
	"i-110_167.sa2", "i-110_168.a2m", "i-110_169.amd", "i-110_170.a2m",
	"i-110_171.amd", "i-110_172.amd", "i-110_173.xad", "i-110_174.amd",
	"i-110_175.amd", "i-110_176.dtm", "i-110_177.amd", "i-110_178.dtm",
	"i-110_179.dtm", "i-110_180.a2m", "i-110_181.dtm", "i-110_182.dtm",
	"i-110_183.a2m", "i-110_184.dtm", "i-110_185.dtm", "i-110_186.a2m",
	"i-110_187.amd", "i-110_188.mtk", "i-110_189.amd", "i-110_190.amd",
	"i-110_191.dtm", "i-110_192.a2m", "i-110_193.amd", "i-110_194.dtm",
	"i-110_195.dtm", "i-110_196.a2m", "i-110_197.sa2", "i-110_198.dtm",
	"i-110_199.amd", "i-110_200.a2m",
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

static bool dir_exists(const std::string &path)
{
	struct stat info;

	int rc = stat(path.c_str(), &info);

	if (rc != 0)
		return false;

	return (info.st_mode & S_IFDIR);
}

// shell-style quoting
static std::string quote(std::string s)
{
#ifdef _WIN32 // windows
	// Quoting commands is a complete mess on windows. While not perfect,
	// this should be good enough for arguments that are parsed by both
	// cmd.exe and the default argv parser. It might not work for other
	// cases (command name, custom arguments parsers, ...). See also
	// https://daviddeley.com/autohotkey/parameters/parameters.htm#WIN
	const char *q = s.find_first_of(" \t") == std::string::npos ? NULL : "^\"";
	std::string::size_type i, pos = 0;

	// add a backslash before each double quote and double the
	// immediately preceding backslashes
	while ((pos = s.find('"', pos)) != std::string::npos) {
		i = pos++;
		do {
			s.insert(i, "\\");
			pos++;
		} while (i > 0 && s[--i] == '\\');
	}
	
	// double the trailing backslashes
	pos = s.find_last_not_of('\\');
	if (pos != std::string::npos) pos++;
	else pos = 0;
	if (pos != s.size()) s.append(s, pos);

	// escape special characters for cmd.exe
	pos = 0;
	while ((pos = s.find_first_of("^<>|&()\"", pos)) != std::string::npos) {
		s.insert(pos, "^");
		pos += 2;
	}
	
	// surround with quotes if necessary
	if (q) return q + s + q;

#elif defined(__unix__) || defined(__unix) || defined(__MACH__) // unix
	// POSIX says a backslash quotes any character except newline, but we
	// use it only for printable ASCII chars that may have special meaning.
#define SH_ESCAPE " !\"#$%&'()*,;<=>?[\\]`{|}~"
	// No need to ever quote these:
#define SH_SAFE "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
		"abcdefghijklmnopqrstuvwxyz+-./:@^_"
	// Everything else will always be enclosed in quotes.

	std::string::size_type unsafe, squote, needq, pos;

	// The simple cases:
	if (s.size() == 0)
		return std::string("''"); // empty string

	unsafe = s.find_first_not_of(SH_SAFE);
	if (unsafe == std::string::npos)
		return s; // needs no quoting at all

	squote = s.find('\'');
	if (squote == std::string::npos)
		return '\'' + s + '\''; // enclose in single quotes
	if (s.find_first_of("\"\\$`!") == std::string::npos)
		return '"' + s + '"'; // enclose in double quotes

	// More complicated, needs backslashes or mixed quoting:
	needq = s.find_first_not_of(SH_ESCAPE SH_SAFE);
	pos = 0;
	while (unsafe != std::string::npos) {
		if (needq != std::string::npos &&
			(squote == std::string::npos || needq < squote)) {
			// use single quotes for the part including chars that
			// can't use backslashes up to the next single quote,
			// and add a backslash for the quote
			s.insert(pos, "'");
			if (squote == std::string::npos) {
				s += '\'';
				break;
			}
			s.insert(squote + 1, "'\\");
			pos = unsafe = s.find_first_not_of(SH_SAFE, squote + 4);
			squote = s.find('\'', pos);
			needq = s.find_first_not_of(SH_ESCAPE SH_SAFE, pos);
		} else {
			// insert a backslash in front of the next unsafe
			// char, which we know is in SH_ESCAPE
			s.insert(unsafe, "\\");
			if (squote == unsafe)
				squote = s.find('\'', unsafe + 2);
			else if (squote != std::string::npos)
				squote++;
			if (needq != std::string::npos)
				needq++;
			pos = unsafe = s.find_first_not_of(SH_SAFE, unsafe + 2);
		}
	}
#undef SH_SAFE
#undef SH_ESCAPE
#else
	// other operating system
	static bool warned = false;
	if (!warned) {
		std::cerr << "warning: argument quoting not implemented for your OS" << std::endl;
		warned = true;
	}
#endif
	return s;
}

static bool run_test(int argc, const char *const argv[])
{
	const int timeout = 60;     // real time
	const float limit = 3600.f; // simulated playback time

	// For now, a single argument is expected: a file name.
	// Future test cases may provide additional arguments.
	if (argc != 1)
		std::cerr << "Warning: unsupported # of arguments (got "
			<< argc << ", expected 1)\n";

	alarm(timeout);

	SilentTestopl opl;
	// Load test file
	std::cout << "loading " << argv[0] << " ...";
	std::cout.flush();
	CPlayer *p = CAdPlug::factory(argv[0], &opl);
	if (p) {
		std::cout << " done\n";
	} else {
		std::cout << " invalid file\n";
		return false;
	}

	// Output file information
	std::cout << " - type:        " << p->gettype()   << std::endl;
	std::cout << " - title:       " << p->gettitle()  << std::endl;
	std::cout << " - author:      " << p->getauthor() << std::endl;
	std::cout << " - description: " << p->getdesc()   << std::endl;

	// Process the file
	float t = 0;
	while (p->update()) {
		float refresh = p->getrefresh();
		// With a CEmuopl or similar, SAMPLERATE/refresh sound
		// samples are available and can be read with:
		//opl.update(buf, n);

		if (refresh > 0) {
			t += 1.f / refresh;
		} else {
			std::cerr << "   bad refresh: " << refresh << std::endl;
			t += 1.f;
		}
		if (t >= limit) break;
	}
	std::cout << (t >= limit ? " ! give up after " : " - play time: ")
		<< t << " sec." << std::endl;

	delete p;
	return true;
}

static bool test_wrapper(const std::string &cmdprefix, const std::string &file)
{
	std::string cmd = cmdprefix + quote(file);

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

#ifdef __APPLE__
	// Apple's System Integrity Protection drops these environment
	// variables when executing the shell, so in order to run tests from
	// the build dir, we need to restore them.
	static const char *const fwd[] = {"DYLD_LIBRARY_PATH", "LD_LIBRARY_PATH"};
	for (int i = 0; i < sizeof(fwd) / sizeof(fwd[0]); i++) {
		const char *s = getenv(fwd[i]);
		if (!s) continue;

		std::string assignment(fwd[i]);
		assignment += '=';
		assignment += quote(s);

		std::cerr << "info: forwarding " << assignment << std::endl;
		cmd += assignment;
		cmd += ' ';
	}
#endif

	// Optional wrapper executable to run test cases
	const char *wrapper = getenv("stresstest_wrapper");
	if (wrapper) {
		cmd += wrapper;
		cmd += ' ';
	}
	cmd += quote(argv[0]); // Re-exec ourselves
	cmd += " + ";

	// Set path to test case directory
	std::string dir;
	if (const char *s = getenv("testdir")) {
		dir = s;
		int l = strlen(DIR_DELIM);
		if (dir.compare(dir.size() - l, l, DIR_DELIM) != 0)
			dir += DIR_DELIM;
	}
	if (dir_exists(dir + SUBDIR))
		dir += SUBDIR DIR_DELIM;

#ifdef DJGPP
	// DJGPP/DosEMU has weird behaviour when parsing command line arguments (adds path and name of executable as seperate vars)
	// Don't parse the rest by setting argument count to 1
	std::cout << "Warning: Running test with DJGPP, ignoring command line arguments, executing all test files!" << std::endl;
	argc = 1;
#endif
	bool fail = false;
	if (argc < 2) {
		// No files, so run all tests from filelist.
		for (int i = 0; i < filecount; i++)
			fail |= !test_wrapper(cmd, dir + filelist[i]);
	} else {
		// Test the file(s) on the command line
		for (int i = 1; i < argc; i++)
			fail |= !test_wrapper(cmd, strstr(argv[i], DIR_DELIM) ||
				access(argv[i], F_OK) ? argv[i] : dir + argv[i]);
	}

	return fail ? EXIT_FAILURE : EXIT_SUCCESS;
}

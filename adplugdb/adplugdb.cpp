/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (c) 1999 - 2003 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * adplugdb.cpp - AdPlug database maintenance utility
 * Copyright (c) 2002 Riven the Mage <riven@ok.ru>
 * Copyright (c) 2002, 2003 Simon Peter <dn.tlp@gmx.net>
 */

#include <memory.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <binfile.h>

#include "../src/adplug.h"
#include "../src/silentopl.h"
#include "../src/database.h"
#include "getopt.h"

/***** Defines *****/

#define UNKNOWN_FILETYPE	"*** Unknown ***"

// Message urgency levels
#define MSG_PANIC	0
#define MSG_ERROR	1
#define MSG_WARN	2
#define MSG_NOTE	3
#define MSG_DEBUG	4

/***** Global variables *****/

static const struct {
  const char				*typestr;
  CAdPlugDatabase::CRecord::RecordType	type;
} rtypes[] = {
  { "plain", CAdPlugDatabase::CRecord::Plain },
  { "songinfo", CAdPlugDatabase::CRecord::SongInfo },
  { "clockspeed", CAdPlugDatabase::CRecord::ClockSpeed },
  {0}
};

static struct {
  const char *db_file;
  CAdPlugDatabase::CRecord::RecordType rtype;
  int message_level;
} cfg = {
  "adplug.db",
  CAdPlugDatabase::CRecord::Plain,
  MSG_NOTE
};

static CAdPlugDatabase	mydb;
static const char	*program_name;

/***** Functions *****/

static void message(int level, const char *fmt, ...)
{
  va_list argptr;

  if(cfg.message_level < level) return;

  fprintf(stderr, "%s: ", program_name);
  va_start(argptr, fmt);
  vfprintf(stderr, fmt, argptr);
  va_end(argptr);
  fprintf(stderr, "\n");
}

static CAdPlugDatabase::CKey file2key(const char *filename)
{
  binifstream f(filename);

  if(f.error()) {
    message(MSG_ERROR, "can't open specified file -- %s", filename);
    exit(EXIT_FAILURE);
  }

  return CAdPlugDatabase::CKey(f);
}

static void usage()
{
  printf("Usage: %s [options] <command> [arguments]\n\n"
	 "Commands:\n"
	 "  add <files>      Add (and replace) files to database\n"
	 "  list [files]     List files (or everything) from database\n"
	 "  remove <files>   Remove files from database\n"
	 "\n"
	 "Database options:\n"
	 "  -d <file>        Use different database file\n"
	 "  -t <type>        Add as different record type\n"
	 "\n"
	 "Generic options:\n"
	 "  -q               Be more quiet\n"
	 "  -v               Be more verbose\n"
	 "  -h               Display this help\n"
	 "  -V               Display version information\n",
	 program_name);
}

static const std::string file2type(const char *filename)
{
  CPlayers::const_iterator	i;
  CSilentopl			opl;
  CPlayer			*p;

  for(i = CAdPlug::players.begin(); i != CAdPlug::players.end(); i++)
    if((p = (*i)->factory(&opl)))
      if(p->load(filename)) {
	delete p;
	return (*i)->filetype;
      } else
	delete p;

  message(MSG_WARN, "unknown filetype -- %s", filename);
  return UNKNOWN_FILETYPE;
}

static void db_add(const char *filename)
{
  CAdPlugDatabase::CRecord *record = CAdPlugDatabase::CRecord::factory(cfg.rtype);

  if(!record) {
    message(MSG_ERROR, "internal error (not enough memory?)");
    exit(EXIT_FAILURE);
  }

  record->key = file2key(filename);
  record->filetype = file2type(filename);
  if(record->filetype == UNKNOWN_FILETYPE) { delete record; return; }
  if(!record->user_read(std::cin, std::cout)) {
    message(MSG_ERROR, "data entry error");
    exit(EXIT_FAILURE);
  }

  if(mydb.lookup(record->key)) {
    message(MSG_NOTE, "replacing previous record -- %s", filename);
    mydb.wipe();
  }

  if(mydb.insert(record)) {
    message(MSG_NOTE, "added record, file type \"%s\" -- %s",
	    record->filetype.c_str(), filename);
  } else {
    delete record;
    message(MSG_ERROR, "error adding record to database -- %s", filename);
    exit(EXIT_FAILURE);
  }
}

static bool db_resolve(const char *filename)
/* Resolves and lists one entry from the database */
{
  if(mydb.lookup(file2key(filename))) {
    mydb.get_record()->user_write(std::cout);
    return true;
  } else {
    message(MSG_WARN, "no entry in database -- %s", filename);
    return false;
  }
}

static void db_remove(const char *filename)
/* Removes one entry from the database */
{
  if(mydb.lookup(file2key(filename))) {
    mydb.wipe();
    message(MSG_NOTE, "deleted entry -- %s", filename);
  } else
    message(MSG_WARN, "no entry in database, could not delete -- %s", filename);
}

static void copyright()
/* Print copyright notice and version information */
{
  printf("AdPlug database maintenance utility %s\n", CAdPlug::get_version().c_str());
  printf("Copyright (c) 2002 Riven the Mage <riven@ok.ru>\n"
	 "Copyright (c) 2002 Simon Peter <dn.tlp@gmx.net>\n");
}

static void db_error(bool dbokay)
/* Checks if database is open. Exits program otherwise */
{
  if(!dbokay) {	// Database could not be opened
    message(MSG_ERROR, "database could not be opened -- %s", cfg.db_file);
    exit(EXIT_FAILURE);
  }
}

/***** Main program *****/

int main(int argc, char *argv[])
{
  char		opt;
  bool		dbokay;
  unsigned int	i;

  // Extract program name from argv[0]
  program_name = strrchr(argv[0], '/') ? strrchr(argv[0], '/') + 1 : argv[0];

  // Parse options
  while((opt = getopt(argc, argv, "d:t:qvhV")) != -1)
    switch(opt) {
    case 'd': cfg.db_file = optarg; break;		// Set database file
    case 't': // Different record type
      for(i = 0; rtypes[i].typestr; i++)
	if(!strcmp(rtypes[i].typestr, optarg)) {
	  cfg.rtype = rtypes[i].type;
	  break;
	}

      if(!rtypes[i].typestr) {
	message(MSG_ERROR, "unknown record type -- %s", optarg);
	exit(EXIT_FAILURE);
      }
      break;
    case 'q': if(cfg.message_level) cfg.message_level--; break;	// Be more quiet
    case 'v': cfg.message_level++; break;	       	// Be more verbose
    case 'h': usage(); exit(EXIT_SUCCESS); break;	// Display help
    case 'V': copyright(); exit(EXIT_SUCCESS); break;	// Display version
    case '?': exit(EXIT_FAILURE);
    }

  // Load database file
  dbokay = mydb.load(cfg.db_file);

  // Check for commands
  if(argc == optind) {
    fprintf(stderr, "%s: need a command\n", program_name);
    fprintf(stderr, "Try '%s -h' for more information.\n", program_name);
    exit(EXIT_FAILURE);
  }

  // Parse commands
  if(!strcmp(argv[optind], "add")) {	// Add file to database
    if(++optind < argc) {
      for(;optind < argc; optind++)
	db_add(argv[optind]);
      mydb.save(cfg.db_file);
    } else {
      message(MSG_ERROR, "add -- missing file argument");
      exit(EXIT_FAILURE);
    }
  } else
  if(!strcmp(argv[optind], "list")) {	// List (files from) database
    db_error(dbokay);
    if(++optind < argc) {
      for(;optind < argc; optind++)
	db_resolve(argv[optind]);
    } else {
      mydb.goto_begin();
      do {
	mydb.get_record()->user_write(std::cout);
	printf("\n");
      } while(mydb.go_forward());
    }
  } else
  if(!strcmp(argv[optind], "remove")) {	// Remove files from database
    db_error(dbokay);
    if(++optind < argc) {
      for(;optind < argc; optind++)
	db_remove(argv[optind]);
      mydb.save(cfg.db_file);
    } else {
      message(MSG_ERROR, "remove -- missing file argument");
      exit(EXIT_FAILURE);
    }
  } else {
    message(MSG_ERROR, "unknown command -- %s", argv[optind]);
    exit(EXIT_FAILURE);
  }
}

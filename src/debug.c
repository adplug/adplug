/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2002 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * debug.h - AdPlug Debug Logger
 * Copyright (c) 2002 RtM <riven@ok.ru>
 * Copyright (c) 2002 Simon Peter <dn.tlp@gmx.net>
 */

#ifdef DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static char *logfilename = NULL;

void LogFile(const char *filename)
{
  if(logfilename) free(logfilename);
  logfilename = (char *)malloc(strlen(filename)+1);
  strcpy(logfilename,filename);
}

void LogWrite(const char *fmt, ...)
{
  char logbuffer[256];
  va_list argptr;
  FILE *log;

  va_start(argptr, fmt);
  vsprintf(logbuffer, fmt, argptr);
  va_end(argptr);

  if(logfilename) {
    log = fopen(logfilename,"at");
    fprintf(log,logbuffer);
    fclose(log);
  } else
    fprintf(stderr,logbuffer);
}

#else

void LogFile(char *filename) { }
void LogWrite(char *fmt, ...) { }

#endif

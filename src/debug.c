/*
  AdPlug Debug Logger, by RtM <riven@ok.ru>
*/

#include <stdio.h>
#include <stdarg.h>

static FILE *f_log;

void LogOpen(char *log)
{
  f_log = fopen(log,"wt");
}

void LogWrite(char *fmt, ...)
{
  char logbuffer[256];

  va_list argptr;
  va_start(argptr, fmt);
  vsprintf(logbuffer, fmt, argptr);
  va_end(argptr);

  fprintf(f_log,logbuffer);
}

void LogClose(void)
{
  fclose(f_log);
}

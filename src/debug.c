/*
  AdPlug Debug Logger, by RtM <riven@ok.ru>
*/

#ifdef _DEBUG

#include <stdio.h>
#include <stdarg.h>

static FILE *f_log;

static void LogOpen(char *log)
{
  f_log = fopen(log,"wt");
}

static void LogWrite(char *fmt, ...)
{
  char logbuffer[256];

  va_list argptr;
  va_start(argptr, fmt);
  vsprintf(logbuffer, fmt, argptr);
  va_end(argptr);

  fprintf(f_log,logbuffer);
}

static void LogClose(void)
{
  fclose(f_log);
}

#endif // _DEBUG

/*
  AdPlug Debug Logger, by RtM <riven@ok.ru>
*/

#ifndef H_DEBUG
#define H_DEBUG

extern "C"
{
	void LogOpen(char *log);
	void LogWrite(char *fmt, ...);
	void LogClose(void);
}

#endif

/*
 * debug.h - Enables console output for Win32-based applications, by Simon Peter (dn.tlp@gmx.net)
 *
 * Include this header to enable run-time debugging output within AdPlug winamp plugin.
 * Call debug_init() first, before doing any ordinary console output.
 * debug_init() opens a console window and initializes console output.
 */

//#include <stdio.h>
#include <io.h>
#include <windows.h>
#include <fcntl.h>
#include <conio.h>

void debug_init(void)
{
	int hCrt;
	FILE *hf;

	AllocConsole();
	hCrt = _open_osfhandle((long) GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);
	hf = _fdopen( hCrt, "w" );
	*stdout = *hf;
	setvbuf( stdout, NULL, _IONBF, 0 );
	puts("Debug Started.");
}

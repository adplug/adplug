/*
 * AdPlay - AdPlug DOS Player, by Simon Peter (dn.tlp@gmx.net)
 */

#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include <malloc.h>
#include <direct.h>

#include "txtgfx.h"
#include "window.h"

#include "timer.h"
/*#define std
#define string	String */
#include "adplug.h"
#include "realopl.h"

#define timer_newfreq(freq)	tmSetNewRate((int)(1192737/freq))

// global defines
#define ADPLAYVERS	"AdPlay v1.0"	// AdPlay version string
#define DEFSTACK		(32*1024)		// default stack size for timer-replay in k's
#define DEFPORT		0x388			// default AdLib port
#define MAXINILINE	256			// max. length of a line in the INI-File, incl. 0-char
#define MAXVAR		20			// max. length of a variable name
#define COLORFILE		"colors.ini"	// filename of colors definition file

// global variables
CPlayer		*p = 0;
bool			playing = false;
CTxtWnd		titlebar(70,3,0,0);
CListWnd		filesel(19,20,0,4);
CTxtWnd		infownd(54,20,21,4);
unsigned char	backcol=7;

void poll_player(void)
{
	static float oldfreq=1;

	if(!playing)
		return;

	p->update();
	if(oldfreq != p->getrefresh()) {
		oldfreq = p->getrefresh();
		timer_newfreq(oldfreq);
	}
}

void listfiles(CListWnd &fl)
/* takes a CListWnd and rebuilds it with a filelist of the current working directory */
{
	DIR *dirp,*direntp;
	char *wd;

	fl.removeall();
	wd = getcwd(NULL,0);
	dirp = opendir(wd);
	while(direntp = readdir(dirp))	// add directories
		if(direntp->d_attr & _A_SUBDIR)
			fl.additem(direntp->d_name);
	rewinddir(dirp);
	while(direntp = readdir(dirp))	// add files
		if(!(direntp->d_attr & _A_SUBDIR))
			fl.additem(direntp->d_name);
	closedir(dirp);
	free(wd);
}

void listcolors(char *fn, CListWnd &cl)
/* takes a CListWnd and rebuilds it with the color-sections list of the given fn */
{
	ifstream	f(fn,ios::in | ios::nocreate);
	char		vglsec[MAXINILINE],var[MAXVAR],*dummy;

	if(!f.is_open())			// file not found?
		return;

	cl.removeall();
	do {					// list sections
		f.getline(vglsec,MAXINILINE);
		if(sscanf(vglsec," [%s",var) && *vglsec) {
			dummy = strrchr(var,']');
			*dummy = '\0';
			cl.additem(var);
		}
	} while(!f.eof());
}

void display_help(CTxtWnd &w)
{
	w.clear();
	w.setcaption("Help");
	w.puts("Keyboard Control:\n\n"
		 "Up/Down   - Select in Menu\n"
		 "PgUp/PgDn - Scroll Info Window\n"
		 "ESC       - Exit to DOS\n"
		 "F1        - Help\n"
		 "D         - Shell to DOS\n"
		 "C         - Change Colormap\n"
		 "I         - Refresh Song Info");
	w.redraw();
}

bool isdirectory(char *str)
{
	unsigned int attr;

	_dos_getfileattr(str,&attr);
	if(attr & _A_SUBDIR)
		return true;
	else
		return false;
}

bool loadcolors(char *fn, char *section)
{
	ifstream		f(fn,ios::in | ios::nocreate);
	char			vglsec[MAXINILINE],var[MAXVAR],*dummy;
	unsigned int	val;

	if(!f.is_open())			// file not found?
		return false;

	do {					// search section
		f.getline(vglsec,MAXINILINE);
		sscanf(vglsec," [%s",var);
		dummy = strrchr(var,']');
		*dummy = '\0';
	} while(strcmp(section,var) && !f.eof());
	if(strcmp(section,var))	// section not found
		return false;

	do {					// parse section entries
		f >> var; f.ignore(MAXINILINE,'='); f >> val;
		if(!strcmp(var,"Background")) backcol = val;
		if(!strcmp(var,"titleBorder")) titlebar.setcolor(titlebar.colBorder,val);
		if(!strcmp(var,"titleIn")) titlebar.setcolor(titlebar.colIn,val);
		if(!strcmp(var,"titleCaption")) titlebar.setcolor(titlebar.colCaption,val);
		if(!strcmp(var,"fileselBorder")) filesel.setcolor(filesel.colBorder,val);
		if(!strcmp(var,"fileselSelect")) filesel.setcolor(filesel.colSelect,val);
		if(!strcmp(var,"fileselUnselect")) filesel.setcolor(filesel.colUnselect,val);
		if(!strcmp(var,"fileselCaption")) filesel.setcolor(filesel.colCaption,val);
		if(!strcmp(var,"infowndBorder")) infownd.setcolor(infownd.colBorder,val);
		if(!strcmp(var,"infowndIn")) infownd.setcolor(titlebar.colIn,val);
		if(!strcmp(var,"infowndCaption")) infownd.setcolor(infownd.colCaption,val);
	} while(!strchr(var,'[') && !f.eof());

	return true;
}

void select_colors()
{
	CListWnd	colwnd(MAXVAR,10);
	char		inkey=0;
	bool		ext;

	colwnd.setcaption("Colormaps");
	colwnd.center();
	listcolors(COLORFILE,colwnd);
	colwnd.redraw();

	do {
		if(kbhit())
			if(!(inkey = toupper(getch()))) {
				ext = true;
				inkey = toupper(getch());
			} else
				ext = false;
		else
			inkey = 0;

		if(ext)	// handle all extended keys
			switch(inkey) {
			case 72:	// [Up Arrow] - menu up
				colwnd.select_prev();
				colwnd.redraw();
				break;
			case 80:	// [Down Arrow] - menu down
				colwnd.select_next();
				colwnd.redraw();
				break;
			}
		else		// handle all normal keys
			switch(inkey) {
			case 13:	// [Return] - select colorsheme
				loadcolors(COLORFILE,colwnd.getitem(colwnd.getselection()));
				clearscreen(backcol);
				titlebar.redraw();
				filesel.redraw();
				if(playing)
					infownd.redraw();
				break;
			}
	} while(inkey != 27 && inkey != 13);	// [ESC] - Exit menu
}

void refresh_songinfo(CTxtWnd &w)
{
	char			tmpstr[80];
	unsigned int	i;

	w.clear();
	w.outtext("File Type: "); w.puts(p->gettype());
	w.outtext("Author: "); w.puts(p->getauthor() + "\n");
	sprintf(tmpstr,"Position: %d / %d",p->getorder(),p->getorders()); w.puts(tmpstr);
	sprintf(tmpstr,"Pattern: %d / %d",p->getpattern(),p->getpatterns()); w.puts(tmpstr);
	sprintf(tmpstr,"Row: %d",p->getrow()); w.puts(tmpstr);
	sprintf(tmpstr,"Speed: %d",p->getspeed()); w.puts(tmpstr);
	sprintf(tmpstr,"Timer: %.2f Hz",p->getrefresh()); w.puts(tmpstr);
	sprintf(tmpstr,"Instruments: %d\n",p->getinstruments()); w.puts(tmpstr);
	w.puts("Instrument Names:");
	for(i=0;i<p->getinstruments();i++)
		w.puts(p->getinstrument(i));
}

int main(int argc, char *argv[])
{
	CAdPlug		ap;
	CRealopl		opl(DEFPORT);
	char			inkey=0,*prgdir,*curdir;
	bool			ext,analyzer=false,sinfo=false;

	cout << ADPLAYVERS << ", (c) 2000 - 2001 Simon Peter (dn.tlp@gmx.net)" << endl;

/*	if(!opl.detect()) {
		cout << "No OPL2 detected on port " << hex << DEFPORT << "h!" << endl;
		return 2;
	} */

	if(argc > 1)	// commandline playback
		if(!(p = ap.factory(argv[1],&opl))) {
			cout << "[" << argv[1] << "]: unsupported file type!" << endl;
			return 1;
		} else {
			cout << "Background playback... (type EXIT to stop)" << endl;
			tmInit(poll_player,0xffff,DEFSTACK);
			playing = true;
			_heapshrink();
			system(getenv("COMSPEC"));
			playing = false;
			tmClose();
			delete p;
			opl.init();
			return 0;
		}

	// init
	prgdir = getcwd(NULL,0);
	loadcolors(COLORFILE,"default");
	tmInit(poll_player,0xffff,DEFSTACK);
	setvideomode(3);
	clearscreen(backcol);
	hidecursor();
	titlebar.setcaption(ADPLAYVERS);
	titlebar.puts("     " ADPLAYVERS ", (c) 2000 - 2001 Simon Peter (dn.tlp@gmx.net)");
	titlebar.centerx();
	titlebar.redraw();
	filesel.setcaption("File Selector");
	listfiles(filesel);
	filesel.redraw();

	// main loop
	do {
		if(analyzer) {		// display spectrum analyzer
			// speccy code here...
		}

		if(sinfo && playing) {	// auto-update song info
			refresh_songinfo(infownd);
			infownd.redraw();
		}

		if(kbhit())
			if(!(inkey = toupper(getch()))) {
				ext = true;
				inkey = toupper(getch());
			} else
				ext = false;
		else
			inkey = 0;

		if(ext)	// handle all extended keys
			switch(inkey) {
			case 59:	// [F1] - display help
				display_help(infownd);
				sinfo = false;
				break;
			case 72:	// [Up Arrow] - menu up
				filesel.select_prev();
				filesel.redraw();
				break;
			case 80:	// [Down Arrow] - menu down
				filesel.select_next();
				filesel.redraw();
				break;
			case 73:	// [Page Up] - scroll up file info box
				infownd.scroll_up();
				infownd.redraw();
				break;
			case 81:	// [Page Down] - scroll down file info box
				infownd.scroll_down();
				infownd.redraw();
				break;
			}
		else		// handle all normal keys
			switch(inkey) {
			case 13:	// [Return] - play file / go to directory
				if(isdirectory(filesel.getitem(filesel.getselection()))) {
					chdir(filesel.getitem(filesel.getselection()));
					filesel.selectitem(0);
					listfiles(filesel);
					filesel.redraw();
					break;
				}
				playing = false;
				opl.init();
				if(!(p = ap.factory(filesel.getitem(filesel.getselection()),&opl))) {
					CTxtWnd	errwnd(26,3);
					errwnd.setcaption("Error!");
					errwnd.puts(" Unsupported file type!");
					errwnd.center();
					errwnd.redraw();
					while(!getch());
					errwnd.hide();
					errwnd.redraw();
					break;
				} else {
					char titletxt[80],*title = new char [p->gettitle().length()+1];
					playing = true;
					titlebar.clear();
					strcpy(title,p->gettitle());
					sprintf(titletxt,"Title  : %s",title);
					titlebar.puts(titletxt);
					titlebar.redraw();
					delete [] title;
				}
				// fall through...
			case 'I':	// refresh song info
				infownd.setcaption("Song Info");
				sinfo = true;
				break;
			case 'D':	// shell to DOS
				setvideomode(3);
				showcursor();
				_heapshrink();
				system(getenv("COMSPEC"));
				clearscreen(backcol);
				hidecursor();
				titlebar.redraw();
				listfiles(filesel);
				filesel.redraw();
				break;
			case 'C':
				curdir = getcwd(NULL,0);
				chdir(prgdir);
				select_colors();
				chdir(curdir);
				free(curdir);
				break;
			}
	} while(inkey != 27);	// [ESC] - Exit to DOS

	// deinit
	tmClose();
	if(p) delete p;
	opl.init();
	setvideomode(3);
	showcursor();
	chdir(prgdir);
	free(prgdir);
	return 0;
}

/*
 * AdPlug - Winamp input plugin, by Simon Peter (dn.tlp@gmx.net)
 */

#include <fstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <prsht.h>
#include "resource.h"

#include "adplug.h"							// AdPlug helper object
extern "C" {
#include "in2.h"							// Winamp plugin stuff
#include "frontend.h"						// Winamp frontend IPC stuff
}

// OPL devices
#include "emuopl.h"							// Tatsuyuki's OPL2 emulator
#include "kemuopl.h"						// Ken's OPL2 emulator
#include "realopl.h"						// hardware OPL2
#include "silentopl.h"						// completely silent OPL2

// output IDs (first all emulators, then hardware only, then hardware + emulators in the same order)
enum outputs {emuks, emuts, opl2, opl2emuks, opl2emuts};

// globals
#define ADPLUGVERS		"AdPlug v0.10"		// AdPlug version string
#define WM_WA_MPEG_EOF	WM_USER+2			// post to Winamp at EOF
#define WM_AP_UPDATE	WM_USER+100			// post to FileInfoProc to update window
#define DFLEMU			emuts				// default (safe) emulation mode

// default configuration
#define REPLAYFREQ	44100					// default replay frequency
#define USE16BIT	true					// use 16 bits? 1 = yes, 0 = no
#define STEREO		false					// enable stereo
#define USEHARDWARE	opl2emuts				// try AdLib hardware by default
#define ADLIBPORT	0x388					// standard AdLib base-port
#define	INFPLAY		false					// let winamp rewind songs
#define	FASTSEEK	false					// seek and keep instruments intact
#define NOTEST		false					// test hardware availability
#define FTIGNORE	";mid;"					// file types to ignore (start list and end all entries with ';' !!)
#define PRIORITY	4						// decode thread priority (4 = normal)

typedef struct {
	char *extension;
	bool ignore;
} tFiletypes;

tFiletypes alltypes[] = {
	"laa\0LucasArts AdLib Audio Files (*.LAA)\0",false,
	"mid\0MIDI Audio Files (*.MID)\0",true,
	"cmf\0Creative Music Files (*.CMF)\0",false,
	"sci\0Sierra AdLib Audio Files (*.SCI)\0",false,
	"d00\0EdLib Modules (*.D00)\0",false,
	"s3m\0Screamtracker 3 AdLib Modules (*.S3M)\0",false,
	"sa2\0Surprise! Adlib Tracker 2 Modules (*.SA2)\0",false,
	"raw\0RdosPlay RAW Files (*.RAW)\0",false,
	"mtk\0MPU-401 Trakker Modules (*.MTK)\0",false,
	"amd\0AMUSIC Adlib Tracker Modules (*.AMD)\0",false,
	"rad\0Reality ADlib Tracker Modules (*.RAD)\0",false,
	"a2m\0AdLib Tracker 2 (*.A2M)\0",false,
	"hsp\0Packed HSC-Tracker Modules (*.HSP)\0",false,
	"hsc\0HSC-Tracker Modules (*.HSC)\0",false,
	"imf\0Apogee IMF Files (*.IMF)\0",false,
	"sng\0SNGPlay Files (*.SNG)\0",false,
	"ksm\0Ken Silverman's Music Format (*.KSM)\0",false,
	"m\0Ultima 6 Music Format (*.M)\0",false,
	NULL
};

int prioresolve[] = {THREAD_PRIORITY_IDLE, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL,
					THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST,
					THREAD_PRIORITY_TIME_CRITICAL};

// global variables
In_Module		mod;
char			lastfn[MAX_PATH];					// currently playing file
int				paused;								// pause flag
CAdPlug			ap;									// global AdPlug object
CPlayer			*player;							// global replayer
Copl			*opl;								// global OPL2 chip
CEmuopl			*emuopl;							// global emulated OPL2
CKemuopl		*kemuopl;							// global second emulated OPL2
CRealopl		*realopl=0;							// global real OPL2
int				killDecodeThread=0;					// kill switch for decode thread
HANDLE			thread_handle=INVALID_HANDLE_VALUE; // handle to decode thread
float			decode_pos_ms;						// decode position in ms
int				currentlength;						// song length cache
int				seek_needed;						// seek flag
int				playing;							// currently playing?
int				bequiet=0;							// should player send output to OPL2?
HMIDIOUT		adlibmidi;							// Windows MIDI-Out handle
UINT			timerhandle;						// Windows Multimedia Timer handle
UINT			timerperiod;						// timer resolution cache
int				volcache;							// volume cache for pause
BOOL			isnt;								// 1 = running on Windows NT
unsigned int	subsong=DFLSUBSONG;					// currently playing subsong
unsigned int	maxsubsongs=1;						// maximum number of subsongs in current song
HWND			fidialog=0;							// Handle to File Info modeless Dialog box

// configuration variables
int				replayfreq, nextreplayfreq = REPLAYFREQ, priority = PRIORITY;
bool			use16bit,nextuse16bit = USE16BIT,infplay=INFPLAY,fastseek=FASTSEEK,notest=NOTEST,stereo,nextstereo=STEREO;
enum outputs	usehardware,nextusehardware = USEHARDWARE;
unsigned short	adlibport,nextadlibport = ADLIBPORT;

DWORD WINAPI __stdcall DecodeThread(void *b); // the emulator thread
void CALLBACK TimerThread(UINT wTimerID,UINT msg,DWORD dwUser,DWORD dw1,DWORD dw2);	// the hardware-replay thread

void setvolume(int volume)
{
	if(usehardware < opl2)
		mod.outMod->SetVolume(volume);
	else
		if(realopl)
			realopl->setvolume((int)(63 - volume/(255/63)));
}

char *upstr(char *str)
/* converts a string to all uppercase letters */
{
	unsigned int i;

	for(i=0;i<strlen(str);i++)
		str[i] = toupper(str[i]);

	return str;
}

bool testignore(char *fn)
{
	char *tmpstr;

	if(!strrchr(fn,'.'))
		return false;

	for(int i=0;alltypes[i].extension;i++) {
		tmpstr = (char *)malloc(strlen(alltypes[i].extension)+1);
		if(!strcmp(upstr(strcpy(tmpstr,alltypes[i].extension)),upstr(strrchr(fn,'.')+1))) {
			free(tmpstr);
			if(alltypes[i].ignore)
				return true;
			else
				return false;
		} else
			free(tmpstr);
	}

	return false;
}

// unimplemented plugin functions
void eq_set(int on, char data[10], int preamp) {}
void init() {}

// pause/seek/length/pan handling
void pause() { paused=1; if(usehardware >= opl2) realopl->setquiet(); else mod.outMod->Pause(1); }
void unpause() { paused=0; if(usehardware >= opl2) realopl->setquiet(false); else mod.outMod->Pause(0); }
int ispaused() { return paused; }
int getoutputtime() { if(usehardware < opl2) return mod.outMod->GetOutputTime(); else return (int)decode_pos_ms; }
void setoutputtime(int time_in_ms) { seek_needed = time_in_ms; }	// hand over to play thread
void setpan(int pan) { if(usehardware < opl2) mod.outMod->SetPan(pan); }
int getlength() { return currentlength; }	// return cached length

/* Dialog box functions */
BOOL APIENTRY AboutBoxProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT		ps;
	HDC				wdc,bdc;
	HANDLE			bm;
	COLORREF		tp,sp;
	unsigned int	x,y;

	switch (message) {
	case WM_INITDIALOG:
		SetDlgItemText(hwndDlg,IDC_ABOUT,
		 ADPLUGVERS ", (c)'99-2001 Simon Peter (dn.tlp@gmx.net) et al.");
		return TRUE;

	case WM_PAINT:
		// draw transparent adplug bitmap
		wdc = BeginPaint(hwndDlg,&ps);
		bdc = CreateCompatibleDC(wdc);
		bm = LoadImage(mod.hDllInstance,MAKEINTRESOURCE(IDB_LOGO),IMAGE_BITMAP,0,0,LR_DEFAULTCOLOR);
		SelectObject(bdc,bm);
		tp = GetPixel(bdc,0,0);
		sp = GetPixel(wdc,12,12);
		for(x=0;x<69;x++)	// make bitmap transparent
			for(y=0;y<35;y++)
				if(GetPixel(bdc,x,y) == tp)
					SetPixel(bdc,x,y,sp);
		BitBlt(wdc,12,12,69,35,bdc,0,0,SRCCOPY);
		DeleteObject(bm);
		ReleaseDC(hwndDlg,bdc);
		EndPaint(hwndDlg,&ps);
		return TRUE;

	case WM_COMMAND:					// commands processing
		switch(LOWORD(wParam)) {
		case IDOK:						// OK button pressed?
			EndDialog(hwndDlg,wParam);	// close Window
			return TRUE;
		}
	}
	return FALSE;
}

BOOL APIENTRY FileInfoProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	unsigned int	subsongpos,i,spos=0;		// subsong slider position cache
	static CPlayer	*p;
	char			tmpstr[10];
	std::string		str;

	switch (message) {
	case WM_INITDIALOG:
		p = (CPlayer *)lParam;	// get player
		// set title/author/type states
		SetDlgItemText(hwndDlg,IDC_TITLE,p->gettitle().c_str());
		SetDlgItemText(hwndDlg,IDC_AUTHOR,p->getauthor().c_str());
		SetDlgItemText(hwndDlg,IDC_TYPE,p->gettype().c_str());

		// set "song description" state
		str = p->getdesc();					// convert ANSI \n to Windows \r\n
		while((spos = str.find('\n',spos)) != str.npos) {
			str.insert(spos,"\r");
			spos += 2;
		}
		SetDlgItemText(hwndDlg,IDC_SONGINFO,str.c_str());

		// set "instrument names" state
		str.erase();
		for(i=0;i<p->getinstruments();i++) {
			if(i < 9)
				sprintf(tmpstr,"0%u - ",i+1);
			else
				sprintf(tmpstr,"%u - ",i+1);
			str += tmpstr + p->getinstrument(i);
			if(i < p->getinstruments() - 1)
				str += "\r\n";
		}
		SetDlgItemText(hwndDlg,IDC_INSTNAMES,str.c_str());

		// set "subsong selection" state
		SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_SETRANGE,(WPARAM)FALSE,(LPARAM)MAKELONG(1,maxsubsongs));
		SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)(LONG)subsong + 1);
		SetDlgItemInt(hwndDlg,IDC_SUBSONGMIN,1,FALSE);
		SetDlgItemInt(hwndDlg,IDC_SUBSONGMAX,maxsubsongs,FALSE);
		SetDlgItemInt(hwndDlg,IDC_SUBSONGPOS,subsong + 1,FALSE);

		// set "song info" state
		SetDlgItemInt(hwndDlg,IDC_INSTS,p->getinstruments(),FALSE);
		// fall through...

	case WM_AP_UPDATE:
		// update "song info" state
		sprintf(tmpstr,"%u / %u",p->getorder(),p->getorders()); SetDlgItemText(hwndDlg,IDC_POSITION,tmpstr);
		sprintf(tmpstr,"%u / %u",p->getpattern(),p->getpatterns()); SetDlgItemText(hwndDlg,IDC_PATTERN,tmpstr);
		SetDlgItemInt(hwndDlg,IDC_ROW,p->getrow(),FALSE);
		SetDlgItemInt(hwndDlg,IDC_SPEED,p->getspeed(),FALSE);
		sprintf(tmpstr,"%.2f Hz",p->getrefresh()); SetDlgItemText(hwndDlg,IDC_TIMER,tmpstr);
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:						// OK button pressed?
		case IDCANCEL:					// Close button pressed?
			DestroyWindow(hwndDlg);		// close Window
			fidialog = 0;
			return TRUE;
		}

	case WM_HSCROLL:
		switch(GetDlgCtrlID((HWND)lParam)) {
		case IDC_SUBSONGSLIDER:
			subsongpos = (unsigned int)SendDlgItemMessage(hwndDlg,IDC_SUBSONGSLIDER,TBM_GETPOS,0,0);
			if(subsongpos - 1 != subsong) {	// new subsong?
				SetDlgItemInt(hwndDlg,IDC_SUBSONGPOS,subsongpos,FALSE);	// update subsong number display
				subsong = subsongpos - 1;	// set new subsong
				SendMessage(mod.hMainWindow,WM_COMMAND,WINAMP_BUTTON2,0);	// trick to make winamp read the new songlength
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL APIENTRY GeneralConfigProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char bufstr[5];
	unsigned long buf;
	// mirror config vars
	static int mnextreplayfreq,mpriority;
	static bool mnextuse16bit,minfplay,mfastseek,mnotest,mnextstereo;
	static unsigned short mnextadlibport;
	static enum outputs mnextusehardware;

	switch (message) {
	case WM_NOTIFY:
		switch(((NMHDR FAR *) lParam)->code) {
		case PSN_KILLACTIVE:
			// check "resolution"
			if(IsDlgButtonChecked(hwndDlg,IDC_QUALITY8) == BST_CHECKED) mnextuse16bit = false;
			if(IsDlgButtonChecked(hwndDlg,IDC_QUALITY16) == BST_CHECKED) mnextuse16bit = true;

			// check "frequency"
			if(IsDlgButtonChecked(hwndDlg,IDC_RATE1) == BST_CHECKED) mnextreplayfreq = 11025;
			if(IsDlgButtonChecked(hwndDlg,IDC_RATE2) == BST_CHECKED) mnextreplayfreq = 22050;
			if(IsDlgButtonChecked(hwndDlg,IDC_RATE3) == BST_CHECKED) mnextreplayfreq = 44100;
			if(IsDlgButtonChecked(hwndDlg,IDC_RATE4) == BST_CHECKED) mnextreplayfreq = 48000;
			if(IsDlgButtonChecked(hwndDlg,IDC_RATE5) == BST_CHECKED)
				mnextreplayfreq = GetDlgItemInt(hwndDlg,IDC_CUSTOMRATE,NULL,FALSE);

			// check "hardware"
			if(IsDlgButtonChecked(hwndDlg,IDC_HARDWAREEMU) == BST_CHECKED) mnextusehardware = emuts;
			if(IsDlgButtonChecked(hwndDlg,IDC_HARDWAREEMU2) == BST_CHECKED) mnextusehardware = emuks;
			if(IsDlgButtonChecked(hwndDlg,IDC_HARDWAREOPL2) == BST_CHECKED) mnextusehardware = opl2;

			// check "OPL2 Port"
			GetDlgItemText(hwndDlg,IDC_ADLIBPORT,bufstr,5);
			sscanf(bufstr,"%x",&buf);
			mnextadlibport = (unsigned short)buf;

			// check "options"
			if(IsDlgButtonChecked(hwndDlg,IDC_AUTOEND) == BST_CHECKED) minfplay = false; else minfplay = true;
			if(IsDlgButtonChecked(hwndDlg,IDC_FASTSEEK) == BST_CHECKED) mfastseek = true; else mfastseek = false;
			if(IsDlgButtonChecked(hwndDlg,IDC_NOTEST) == BST_CHECKED) mnotest = true; else mnotest = false;

			// check "channels"
			if(IsDlgButtonChecked(hwndDlg,IDC_MONO) == BST_CHECKED) mnextstereo = false;
			if(IsDlgButtonChecked(hwndDlg,IDC_STEREO) == BST_CHECKED) mnextstereo = true;

			// check "priority"
			mpriority = (int)SendDlgItemMessage(hwndDlg,IDC_PRIORITY,TBM_GETPOS,0,0);

			return TRUE;

		case PSN_APPLY:
			// apply mirror vars
			nextuse16bit = mnextuse16bit;
			nextreplayfreq = mnextreplayfreq;
			infplay = minfplay;
			fastseek = mfastseek;
			notest = mnotest;
			nextadlibport = mnextadlibport;
			nextusehardware = mnextusehardware;
			nextstereo = mnextstereo;
			priority = mpriority;
			return TRUE;

		case PSN_SETACTIVE:
			if(mnextuse16bit)			// set "resolution" state
				CheckRadioButton(hwndDlg,IDC_QUALITY8,IDC_QUALITY16,IDC_QUALITY16);
			else
				CheckRadioButton(hwndDlg,IDC_QUALITY8,IDC_QUALITY16,IDC_QUALITY8);

			switch(mnextreplayfreq) {	// set "frequency" state
			case 11025: CheckRadioButton(hwndDlg,IDC_RATE1,IDC_RATE5,IDC_RATE1); break;
			case 22050: CheckRadioButton(hwndDlg,IDC_RATE1,IDC_RATE5,IDC_RATE2); break;
			case 44100: CheckRadioButton(hwndDlg,IDC_RATE1,IDC_RATE5,IDC_RATE3); break;
			case 48000: CheckRadioButton(hwndDlg,IDC_RATE1,IDC_RATE4,IDC_RATE4); break;
			default: CheckRadioButton(hwndDlg,IDC_RATE1,IDC_RATE5,IDC_RATE5);
					 SetDlgItemInt(hwndDlg,IDC_CUSTOMRATE,mnextreplayfreq,FALSE);
			}

			switch(mnextusehardware) {		// set "hardware" state
			case emuts: CheckRadioButton(hwndDlg,IDC_HARDWAREEMU,IDC_HARDWAREOPL2,IDC_HARDWAREEMU); break;
			case emuks: CheckRadioButton(hwndDlg,IDC_HARDWAREEMU,IDC_HARDWAREOPL2,IDC_HARDWAREEMU2); break;
			case opl2: CheckRadioButton(hwndDlg,IDC_HARDWAREEMU,IDC_HARDWAREOPL2,IDC_HARDWAREOPL2); break;
			}

			SetDlgItemText(hwndDlg,IDC_ADLIBPORT,_itoa(mnextadlibport,bufstr,16));	// set "OPL2 Port" state

			// set "options" state
			if(!minfplay) CheckDlgButton(hwndDlg,IDC_AUTOEND,BST_CHECKED);
			if(mfastseek) CheckDlgButton(hwndDlg,IDC_FASTSEEK,BST_CHECKED);
			if(mnotest) CheckDlgButton(hwndDlg,IDC_NOTEST,BST_CHECKED);

			// set "channels" state
			switch(mnextstereo) {
			case 0: CheckRadioButton(hwndDlg,IDC_MONO,IDC_STEREO,IDC_MONO); break;
			case 1: CheckRadioButton(hwndDlg,IDC_MONO,IDC_STEREO,IDC_STEREO); break;
			}

			// set "priority" state
			SendDlgItemMessage(hwndDlg,IDC_PRIORITY,TBM_SETRANGE,(WPARAM)FALSE,(LPARAM)MAKELONG(1,7));
			SendDlgItemMessage(hwndDlg,IDC_PRIORITY,TBM_SETPOS,(WPARAM)TRUE,(LPARAM)(LONG)mpriority);

			return TRUE;
		}

	case WM_INITDIALOG:
		// init mirror vars
		mnextuse16bit = nextuse16bit;
		mnextreplayfreq = nextreplayfreq;
		minfplay = infplay;
		mfastseek = fastseek;
		mnotest = notest;
		mnextadlibport = nextadlibport;
		mnextusehardware = nextusehardware;
		mnextstereo = nextstereo;
		mpriority = priority;
		return TRUE;
	}
	return FALSE;
}

BOOL APIENTRY FormatConfigProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static	tFiletypes *mft;
	int		i;

	switch (message) {
	case WM_NOTIFY:
		switch(((NMHDR FAR *) lParam)->code) {
		case PSN_KILLACTIVE:
			// get dialog controls
			for(i=0;i<SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_GETCOUNT,0,0);i++)
				if(SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_GETSEL,i,0))
					mft[i].ignore = false;
				else
					mft[i].ignore = true;

			SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_RESETCONTENT,0,0);	// clear Listbox
			return TRUE;

		case PSN_APPLY:
			// apply mirror vars
			memcpy(alltypes,mft,sizeof(alltypes));
			free(mft);
			return TRUE;

		case PSN_SETACTIVE:
			// set dialog controls
			for(i=0;alltypes[i].extension;i++)
					SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_SETSEL,(BOOL)!mft[i].ignore,
						SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_ADDSTRING,0,(LPARAM)mft[i].extension));
			return TRUE;
		}

	case WM_INITDIALOG:
		// init mirror vars
		mft = (tFiletypes *)malloc(sizeof(alltypes));
		memcpy(mft,alltypes,sizeof(alltypes));
		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_FTSELALL:	// select all items
			SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_SETSEL,TRUE,-1);
			return TRUE;
		case IDC_FTSELNONE:	// select no items
			SendDlgItemMessage(hwndDlg,IDC_FORMATLIST,LB_SETSEL,FALSE,-1);
			return TRUE;
		}
	}

	return FALSE;
}

/**************/

void config(HWND hwndParent)
{
	PROPSHEETPAGE	psp[2];
	PROPSHEETHEADER	psh;

	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_DEFAULT;
	psp[0].hInstance = mod.hDllInstance;
	psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPGENERAL);
	psp[0].pfnDlgProc = (DLGPROC)GeneralConfigProc;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_DEFAULT;
	psp[1].hInstance = mod.hDllInstance;
	psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PROPFMT);
	psp[1].pfnDlgProc = (DLGPROC)FormatConfigProc;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
	psh.hwndParent = hwndParent;
	psh.pszCaption = (LPSTR) ADPLUGVERS " Configuration";
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.nStartPage = 0;
	psh.ppsp = psp;

	PropertySheet(&psh);

	if(nextadlibport != adlibport || (nextusehardware > usehardware && nextusehardware >= opl2))
		if(isnt && !notest) {
			nextusehardware = DFLEMU;
			MessageBox(hwndParent,"OPL2 Hardware replay is disabled under Windows NT! Switching to emulation mode.",ADPLUGVERS,
				MB_OK | MB_ICONERROR);
		} else {
			CRealopl	tmpadl(nextadlibport);
			if(!tmpadl.detect() && !notest) {
				nextusehardware = DFLEMU;
				MessageBox(hwndParent,"No OPL2 detected on specified port! Switching to emulation mode.",ADPLUGVERS,
					MB_OK | MB_ICONERROR);
			}
		}
		if((nextusehardware < opl2 && usehardware >= opl2) || (nextusehardware >= opl2 && usehardware < opl2))
			MessageBox(hwndParent,"Switching between emulated and hardware replay requires Winamp to be restarted before "
				"the changes take effect.",ADPLUGVERS,MB_OK | MB_ICONWARNING);
}

void about(HWND hwndParent)
{ DialogBox(mod.hDllInstance,MAKEINTRESOURCE(IDD_ABOUTBOX),hwndParent,(DLGPROC)AboutBoxProc); }

int infoDlg(char *fn, HWND hwnd)
{
	if(fidialog)
		return 0;

	if(playing && !strcmp(fn,lastfn))
		fidialog = CreateDialogParam(mod.hDllInstance,MAKEINTRESOURCE(IDD_FILEINFO),hwnd,(DLGPROC)FileInfoProc,(LPARAM)player);
	else {
		CSilentopl	sopl;
		CPlayer		*p = ap.factory(fn,&sopl);
		if(p) {
			fidialog = CreateDialogParam(mod.hDllInstance,MAKEINTRESOURCE(IDD_FILEINFO),hwnd,(DLGPROC)FileInfoProc,(LPARAM)p);
			delete p;
		}
	}

	return 0;
}

char *getsongtitle(char *title,char *filename)
{
	CSilentopl	sopl;
	CPlayer *p = ap.factory(filename,&sopl);

	strcpy(title,strrchr(filename,'\\')+1);
	if(!p) return title;
	if(!p->gettitle().empty()) strcpy(title,p->gettitle().c_str());
	delete p;
	return title;
}

int getsonglength(char *filename, int subsng)
{
	int slen;

	CSilentopl	sopl;
	CPlayer *p = ap.factory(filename,&sopl);
	if(!p) return 0;
	slen = ap.songlength(p,subsng);	// get songlength
	delete p;
	return slen;
}

void getfileinfo(char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename) {	// currently playing file
		if (length_in_ms)
			*length_in_ms = getlength();
		if (title)
			if(playing) {			// are we playing right now?
				if(!player->gettitle().empty())
					strcpy(title,player->gettitle().c_str());
				else
					strcpy(title,strrchr(lastfn,'\\')+1);
			} else
				title = getsongtitle(title,filename);
	} else {						// some other file
		if (length_in_ms)
			*length_in_ms = getsonglength(filename,DFLSUBSONG);
		if (title)
			title = getsongtitle(title,filename);
	}
}

int isourfile(char *fn)
{
	if(!strcmp(fn,"hi.mp3"))
		return 0;

	if(testignore(fn))
		return 0;

	CSilentopl	sopl;
	CPlayer *p = ap.factory(fn,&sopl);
	if(p) {
		delete p;
		return 1;
	} else
		return 0;
}

void getinifilename(char *inifile,int len)
{
	GetModuleFileName(GetModuleHandle("in_adlib"),inifile,len);
	inifile[strrchr(inifile,'\\') - inifile] = '\0';
	strcat(inifile,"\\plugin.ini");
}

void quit()
{
	char	*bufstr,*ftignore;	// buffer string, file type ignoration list
	char	inifile[MAX_PATH];	// path to plugin.ini
	int		i,ftsize=0;

	bufstr = (char *) malloc(10);
	// store configuration
	getinifilename(inifile,MAX_PATH);	// retrieve full path to ini-file
	WritePrivateProfileString("AdPlug","ReplayFreq",_itoa(nextreplayfreq,bufstr,10),inifile);
	WritePrivateProfileString("AdPlug","Use16Bit",_itoa(nextuse16bit,bufstr,10),inifile);
	WritePrivateProfileString("AdPlug","UseHardware",_itoa(nextusehardware,bufstr,10),inifile);
	WritePrivateProfileString("AdPlug","AdlibPort",_itoa(nextadlibport,bufstr,16),inifile);
	WritePrivateProfileString("AdPlug","AutoRewind",_itoa(infplay,bufstr,10),inifile);
	WritePrivateProfileString("AdPlug","FastSeek",_itoa(fastseek,bufstr,10),inifile);
	WritePrivateProfileString("AdPlug","NoTest",_itoa(notest,bufstr,10),inifile);
	WritePrivateProfileString("AdPlug","Stereo",_itoa(nextstereo,bufstr,10),inifile);
	WritePrivateProfileString("AdPlug","Priority",_itoa(priority,bufstr,10),inifile);

	// build up file type ignoration list
	for(i=0;alltypes[i].extension;i++)
		if(alltypes[i].ignore)
			ftsize += strlen(alltypes[i].extension)+2;
	ftignore = (char *)malloc(++ftsize); strcpy(ftignore,";");

	for(i=0;alltypes[i].extension;i++)
		if(alltypes[i].ignore) {
			strcat(ftignore,alltypes[i].extension);
			strcat(ftignore,";");
		}
	WritePrivateProfileString("AdPlug","Ignore",ftignore,inifile);

	free(ftignore);
	free(bufstr);
	free(mod.FileExtensions);	// free supported file types list
}

int play(char *fn)
{
	int				maxlatency;
	unsigned long	tmp;
	unsigned int	i;
	LPMIDIOUTCAPS	midicaps;
	LPTIMECAPS		timecaps;

	// init main replay variables
	if(strcmp(fn,lastfn)) {	// new file?
		subsong = DFLSUBSONG;	// start with default subsong
		if(fidialog) {	// close File Info Box, if open
			DestroyWindow(fidialog);
			fidialog = 0;
		}
	}
	currentlength = getsonglength(fn,subsong);	// get song length
	replayfreq = nextreplayfreq; use16bit = nextuse16bit; stereo = nextstereo;
	if((usehardware >= opl2 && nextusehardware >= opl2) || (usehardware < opl2 && nextusehardware < opl2)) {
		usehardware = nextusehardware;
		adlibport = nextadlibport;
	}
	strcpy(lastfn,fn);							// remember filename
	paused=0;									// not paused
	decode_pos_ms=0.0f;							// begin at 0ms
	seek_needed = -1;							// no seek needed

	// if the OPL is also used for MIDI, check if it is available
	if(usehardware >= opl2 && !notest) {
		adlibmidi = NULL;
		midicaps = (LPMIDIOUTCAPS) malloc(sizeof(MIDIOUTCAPS));
		for(i=0;i<midiOutGetNumDevs();i++)
			if(midiOutGetDevCaps(i,midicaps,sizeof(MIDIOUTCAPS)) == MMSYSERR_NOERROR)
				if(midicaps->wTechnology == MOD_FMSYNTH)
					if(midiOutOpen(&adlibmidi,i,0,0,CALLBACK_NULL) != MMSYSERR_NOERROR) {
						MessageBox(mod.hMainWindow,"The OPL2 chip is already in use by the MIDI sequencer!\n"
						"Please quit all running MIDI applications before going on.",ADPLUGVERS,MB_OK | MB_ICONERROR);
						free(midicaps);
						return 1;	// don't play now
					} else
						break;		// we found it
		free(midicaps);
	}

	// init OPL2 output
	switch(usehardware) {
	case emuks: opl = kemuopl = new CKemuopl(replayfreq,use16bit,stereo); break;
	case emuts: opl = emuopl = new CEmuopl(replayfreq,use16bit,stereo); break;
	case opl2: opl = realopl = new CRealopl(adlibport); break;
	}

	// init file player
	if(!(player = ap.factory(fn,opl))) {
		// error! deinit everything
		delete opl;
		if(usehardware >= opl2 && adlibmidi)
			midiOutClose(adlibmidi);
		return 1;	// don't play now
	}
	maxsubsongs = player->getsubsongs();
	player->rewind(subsong);	// rewind player to right subsong

	// init Winamp
	if(usehardware < opl2) {
		if((maxlatency = mod.outMod->Open(replayfreq,stereo ? 2 : 1,use16bit ? 16 : 8, -1,-1)) < 0) {
			// error! deinit everything
			delete player;
			delete opl;
			if(usehardware >= opl2 && adlibmidi)	// deinit MIDI, if needed
				midiOutClose(adlibmidi);
			return 1;	// don't play now
		}
		mod.outMod->SetVolume(-666);
	} else
		maxlatency = 0;		// hardware-replay has no latency
	mod.SetInfo(9*10000,(int)replayfreq/1000,stereo ? 2 : 1,1);

	if(usehardware != opl2) {	// init vis
		mod.SAVSAInit(maxlatency,replayfreq);
		mod.VSASetInfo(stereo ? 2 : 1,replayfreq);
	};

	if(usehardware < opl2) {
		killDecodeThread = 0;
		timerhandle = 0;
		thread_handle = (HANDLE)CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)DecodeThread,(void *)&killDecodeThread,0,&tmp);
		SetThreadPriority(thread_handle,prioresolve[priority]);
	} else {			// hardware replay uses timer
		timecaps = (LPTIMECAPS) malloc(sizeof(TIMECAPS));
		timeGetDevCaps(timecaps,sizeof(TIMECAPS));
		timerperiod = min(max(timecaps->wPeriodMin,0),timecaps->wPeriodMax);
		free(timecaps);
		timeBeginPeriod(timerperiod);
		timerhandle = timeSetEvent((int)(1000/player->getrefresh()),0,TimerThread,0,TIME_PERIODIC);
	}
	playing = 1;	// starting to play
	return 0;
}

void stop() {
	if(usehardware < opl2 && thread_handle != INVALID_HANDLE_VALUE) {	// stop player thread
		killDecodeThread=1;
		if(WaitForSingleObject(thread_handle,INFINITE) == WAIT_TIMEOUT) {
			MessageBox(mod.hMainWindow,"Error terminating player thread!",ADPLUGVERS,MB_OK);
			TerminateThread(thread_handle,0);
		}
		CloseHandle(thread_handle);
		thread_handle = INVALID_HANDLE_VALUE;
	}

	if(usehardware >= opl2 && timerhandle) {	// stop timer thread
		timeKillEvent(timerhandle);
		timeEndPeriod(timerperiod);
		timerhandle = 0;
	}

	playing = 0;						// not playing anymore
	delete player;						// kill player
	if(usehardware != opl2)
		mod.SAVSADeInit();				// deinit vis stuff
	if(usehardware < opl2)
		mod.outMod->Close();			// close output module
	else
		setvolume(0);
	delete opl;							// kill OPL chip
	if(usehardware >= opl2) realopl = 0;
	if(usehardware >= opl2 && adlibmidi && !notest)	// deinit MIDI, if needed
		midiOutClose(adlibmidi);
}

void CALLBACK TimerThread(UINT wTimerID,UINT msg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
//	int				buffersize;		// size of vis buffer
	static float	oldrefresh;		// caches old refresh value

	if(paused)
		return;

	if(!decode_pos_ms)
		oldrefresh = 0;

	if(seek_needed != -1) {			// is seek needed?
		if(seek_needed < decode_pos_ms) {
			player->rewind(subsong);	// rewind module
			decode_pos_ms = 0.0f;
		}
		// seek to specified position
		realopl->setquiet();
		if(fastseek) realopl->setnowrite();
		while(decode_pos_ms < seek_needed && player->update())
			decode_pos_ms += 1000/player->getrefresh();
		if(fastseek) realopl->setnowrite(false);
		realopl->setquiet(false);
		seek_needed = -1;	// reset seek flag
	}

	if(!player->update() && !infplay) {	// update replayer
		PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
		return;
	}

	if(oldrefresh != player->getrefresh()) {	// adjust timer frequency
		timeKillEvent(timerhandle);
		timerhandle = timeSetEvent((int)(1000/player->getrefresh()),0,TimerThread,0,TIME_PERIODIC);
		oldrefresh = player->getrefresh();
	}

	if(fidialog) // update file info box, if displayed
		PostMessage(fidialog,WM_AP_UPDATE,0,0);

/*	if(usehardware > opl2) {	// update vis
		buffersize = (int)(replayfreq/player->getrefresh());
		if(use16bit) buffersize *= 2;
		tempbuf = (char *) realloc(tempbuf,buffersize);
		emuopl->update(tempbuf,buffersize);
		mod.SAAddPCMData(tempbuf,1,use16bit ? 16 : 8,(int)decode_pos_ms);
		mod.VSAAddPCMData(tempbuf,1,use16bit ? 16 : 8,(int)decode_pos_ms);
	} */
	decode_pos_ms += 1000/player->getrefresh();
}

/*DWORD WINAPI __stdcall DecodeThread(void *b)
{
	short	*tempbuf=NULL,*cpybuf;			// our handover buffer
	int		oldsize,newwrite,newsize,towrite,pos,written,cw,size,nibble=0;	// write buffer sizes
	bool	eos=false;								// end of song flag

	while (!*(int *)b) {			// kill switch
		if(seek_needed != -1) {			// seek needed?
			if(seek_needed < decode_pos_ms) {	// seek backwards?
				player->rewind(subsong);	// rewind module
				decode_pos_ms = 0.0f;
				eos = false;
			}
			// seek to new position
			while(decode_pos_ms < seek_needed && player->update())
				decode_pos_ms += 1000/player->getrefresh();
			mod.outMod->Flush((int)decode_pos_ms);	// tell new position
			seek_needed = -1;						// reset seek flag
		}

		if(eos)
			if (!mod.outMod->IsPlaying()) {	// sound ended?
				PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				break;
			} else {
				Sleep(10);
				continue;
			}

		// do some output
		towrite = 0; pos = 0; size = 0;
		do
			if(player->update() || infplay) {	// update replayer
				newwrite = (int)(replayfreq/player->getrefresh());
				towrite += newwrite;	// how much to write?
				newsize = newwrite; if(use16bit) newsize *= 2; if(stereo) newsize *= 2;
				size += newsize; if(mod.dsp_isactive()) size *= 2;
				if(!pos)
					tempbuf = (short *)realloc(tempbuf,size*2);
				else {
					cpybuf = (short *)malloc(size*2);
					memcpy(cpybuf,tempbuf,oldsize);
					free(tempbuf);
					tempbuf = cpybuf;
				}
				switch(usehardware) {
				case emuts:
					emuopl->update(tempbuf+pos,newwrite);
					break;
				case emuks:
					kemuopl->update(tempbuf+pos,newwrite);
					break;
				}
				oldsize = newsize;
				pos += newwrite;
				decode_pos_ms += 1000/player->getrefresh();
			} else {
				eos = true;
				break;
			}
		while(towrite < 576);
		towrite = mod.dsp_dosamples(tempbuf,towrite,use16bit ? 16 : 8,stereo ? 2 : 1,replayfreq);	// update dsp
		if(use16bit) towrite *= 2; if(stereo) towrite *= 2;
		written = towrite;

		while(written) {
			while(!(cw = mod.outMod->CanWrite()))
				Sleep(10);
			if(cw > written) cw = written;	// just write the rest
			if(cw > 8192) cw = 8192;		// doesn't accept more than 8192 bytes
			mod.outMod->Write((char *)tempbuf+(towrite-written),cw);
			written -= cw;
			if(*(int *)b || seek_needed != -1)
				break;
		}
		if(towrite > (use16bit ? 576 * 2 : 576)) {	// update vis
			mod.SAAddPCMData(tempbuf,stereo ? 2 : 1,use16bit ? 16 : 8,mod.outMod->GetWrittenTime());
			mod.VSAAddPCMData(tempbuf,stereo ? 2 : 1,use16bit ? 16 : 8,mod.outMod->GetWrittenTime());
		}

		if(fidialog) // update file info box, if displayed
			SendMessage(fidialog,WM_AP_UPDATE,0,0);
	}
	free(tempbuf);					// free buffer
	return 0;
} */

DWORD WINAPI __stdcall DecodeThread(void *b)
{
	short	*tempbuf=NULL;				// our handover buffer
	int		towrite,written,cw,size;	// write buffer sizes

	while (!*(int *)b) {			// kill switch
		if(seek_needed != -1) {			// seek needed?
			if(seek_needed < decode_pos_ms) {	// seek backwards?
				player->rewind(subsong);	// rewind module
				decode_pos_ms = 0.0f;
			}
			// seek to new position
			while(decode_pos_ms < seek_needed && player->update())
				decode_pos_ms += 1000/player->getrefresh();
			mod.outMod->Flush((int)decode_pos_ms);	// tell new position
			seek_needed = -1;						// reset seek flag
		}

		if(!player->update() && !infplay)	// update replayer
			if (!mod.outMod->IsPlaying()) {	// sound ended?
				PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				break;
			} else {
				Sleep(10);
				continue;
			}

		// do some output
		towrite = (int)(replayfreq/player->getrefresh());	// how much to write?
		if(use16bit) size = towrite*2; else size = towrite; if(stereo) size *= 2; if(mod.dsp_isactive()) size *= 2;
		tempbuf = (short *)realloc(tempbuf,size);
		switch(usehardware) {
		case emuts:
			emuopl->update(tempbuf,towrite);
			break;
		case emuks:
			kemuopl->update(tempbuf,towrite);
			break;
		}
		towrite = mod.dsp_dosamples(tempbuf,towrite,use16bit ? 16 : 8,stereo ? 2 : 1,replayfreq);	// update dsp
		if(use16bit) towrite *= 2; if(stereo) towrite *= 2;
		written = towrite;

		while(written) {
			while(!(cw = mod.outMod->CanWrite()))
				Sleep(10);
			if(cw > written) cw = written;	// just write the rest
			if(cw > 8192) cw = 8192;		// doesn't accept more than 8192 bytes
			mod.outMod->Write((char *)tempbuf+(towrite-written),cw);
			written -= cw;
			if(*(int *)b || seek_needed != -1)
				break;
		}
		if(towrite > (use16bit ? 576 * 2 : 576)) {	// update vis
			mod.SAAddPCMData(tempbuf,stereo ? 2 : 1,use16bit ? 16 : 8,mod.outMod->GetWrittenTime());
			mod.VSAAddPCMData(tempbuf,stereo ? 2 : 1,use16bit ? 16 : 8,mod.outMod->GetWrittenTime());
		}
		decode_pos_ms += 1000/player->getrefresh();

		if(fidialog) // update file info box, if displayed
			PostMessage(fidialog,WM_AP_UPDATE,0,0);
	}
	free(tempbuf);					// free buffer
	return 0;
}

extern "C" In_Module mod = {
	IN_VER,								// version identifier
	ADPLUGVERS							// plugin name
#ifdef __alpha
	" (AXP)"
#else
	" (x86)"
#endif
	,0,									// hMainWindow
	0,									// hDllInstance
	NULL,								// Registered filetypes
	1,									// is_seekable
	1,									// uses output plugins
	config,								// function pointers...
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	getlength,
	getoutputtime,
	setoutputtime,
	setvolume,
	setpan,								// ...end
	0,0,0,0,0,0,0,0,0,					// vis stuff
	0,0,								// dsp
	eq_set,								// function pointer
	NULL,								// setinfo
	0									// out_mod
};

extern "C" __declspec(dllexport) In_Module *winampGetInModule2()
{
	int				bufvalue;			// buffered value
	char			inifile[MAX_PATH];	// path to plugin.ini
	char			bufstring[5];		// small string buffer
	LPOSVERSIONINFO	vers;				// os version information
	unsigned int	i,extptr=0;			// indices
	char			*strptr;			// pointer inside extensions string
	char			*ftignore;			// file type ignoration list

	// load configuration
	getinifilename(inifile,MAX_PATH);	// retrieve full path to ini-file
	if((bufvalue = GetPrivateProfileInt("AdPlug","ReplayFreq",0,inifile))) nextreplayfreq = bufvalue;
	if((bufvalue = GetPrivateProfileInt("AdPlug","Use16Bit",-1,inifile)) != -1) nextuse16bit = bufvalue ? true : false;
	if((bufvalue = GetPrivateProfileInt("AdPlug","UseHardware",-1,inifile)) != -1) nextusehardware = (enum outputs)bufvalue;
	GetPrivateProfileString("AdPlug","AdlibPort","0",bufstring,5,inifile);
	if(strcmp(bufstring,"0")) sscanf(bufstring,"%x",&nextadlibport);
	if((bufvalue = GetPrivateProfileInt("AdPlug","AutoRewind",-1,inifile)) != -1) infplay = bufvalue ? true : false;
	if((bufvalue = GetPrivateProfileInt("AdPlug","FastSeek",-1,inifile)) != -1) fastseek = bufvalue ? true : false;
	if((bufvalue = GetPrivateProfileInt("AdPlug","NoTest",-1,inifile)) != -1) notest = bufvalue ? true : false;
	if((bufvalue = GetPrivateProfileInt("AdPlug","Stereo",-1,inifile)) != -1) nextstereo = bufvalue ? true : false;
	if((bufvalue = GetPrivateProfileInt("AdPlug","Priority",-1,inifile)) != -1) priority = bufvalue;
	ftignore = (char *)malloc(i=strlen(FTIGNORE)+1);
	while(GetPrivateProfileString("AdPlug","Ignore",FTIGNORE,ftignore,i,inifile) == i - 1)
		ftignore = (char *)realloc(ftignore,++i);

	// parse file type ignoration list
	for(i=0;alltypes[i].extension;i++) {
		strptr = (char *)malloc(strlen(alltypes[i].extension)+3);
		strcpy(strptr,";"); strcat(strptr,alltypes[i].extension); strcat(strptr,";");
		if(strstr(upstr(ftignore),upstr(strptr)))
			alltypes[i].ignore = true;
		else
			alltypes[i].ignore = false;
		free(strptr);
	}
	free(ftignore);

	// build up exported file types list
	for(i=0;alltypes[i].extension;i++)	// determine string size
		for(strptr = alltypes[i].extension;*strptr;strptr += strlen(strptr) + 1)
			if(!alltypes[i].ignore)
				extptr += strlen(strptr) + 1;
			else
				strptr += strlen(strptr) + 1;
	mod.FileExtensions = (char *) malloc(extptr+1); extptr = 0;
	for(i=0;alltypes[i].extension;i++)	// fill string
		for(strptr = alltypes[i].extension;*strptr;strptr += strlen(strptr) + 1)
			if(!alltypes[i].ignore) {
				memcpy(mod.FileExtensions+extptr,strptr,strlen(strptr) + 1);
				extptr += strlen(strptr) + 1;
			} else
				strptr += strlen(strptr) + 1;
	mod.FileExtensions[extptr] = '\0';

	// check platform
	vers = (LPOSVERSIONINFO) malloc(sizeof(OSVERSIONINFO));
	vers->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(vers);
	if(vers->dwPlatformId != VER_PLATFORM_WIN32_NT)
		isnt = FALSE;
	else {
		isnt = TRUE;
		nextusehardware = DFLEMU;	// never use hardware on NT
	}
	free(vers);

	// detect adlib
	if(nextusehardware >= opl2 && !notest) {
		CRealopl	tmpadl(nextadlibport);
		if(!tmpadl.detect())
			nextusehardware = DFLEMU;
		else
			mod.UsesOutputPlug = 0;
	}

	// set default values
	replayfreq = nextreplayfreq;
	usehardware = nextusehardware;
	use16bit = nextuse16bit;
	adlibport = nextadlibport;
	stereo = nextstereo;

	return &mod;
}

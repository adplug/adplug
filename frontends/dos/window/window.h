/*
 * window.h - Basic Textmode Windows, by Simon Peter (dn.tlp@gmx.net)
 */

#include <string.h>

#define SCREEN_MAXX	80
#define SCREEN_MAXY	25

#define MAXCAPTION	30
#define MAXSTRING		1024
#define MAXITEMS		100
#define MAXITEMSTRING	100
#define MAXCOLORS		3

#define DEFWNDSIZEX	20
#define DEFWNDSIZEY	20
#define DEFWNDPOSX	0
#define DEFWNDPOSY	0
#define DEFTXTBUFSIZE	1024

class CWindow
{
public:
	CWindow(unsigned char nsizex = DEFWNDSIZEX, unsigned char nsizey = DEFWNDSIZEY, unsigned char nx = DEFWNDPOSX, unsigned char ny = DEFWNDPOSY);
	~CWindow();

	void setcaption(char *newcap)					// sets new window caption
	{ strncpy(caption,newcap,MAXCAPTION); };
	char *getcaption()						// returns current window caption
	{ return caption; };

	void setxy(unsigned char newx, unsigned char newy)	// sets new on-screen x/y position
	{ x = newx; y = newy; };
	void resize(unsigned char newx, unsigned char newy)	// resizes the window
	{ sizex = newx; sizey = newy; };
	void centerx()							// centers the window x-wise on screen
	{ x = (unsigned char)((SCREEN_MAXX - sizex) / 2); };
	void centery()							// centers the window y-wise on screen
	{ y = (unsigned char)((SCREEN_MAXY - sizey) / 2); };
	void center()							// centers the window on screen
	{ centerx(); centery(); };

	enum Color {colBorder, colIn, colCaption};

	void setcolor(Color c, unsigned char v)			// sets window color
	{ color[c] = v; };
	unsigned char getcolor(Color c)				// returns window color
	{ return color[c]; };

	void show(void)							// shows the window
	{ visible = true; };
	void hide(void)							// hides the window
	{ visible = false; };

	void redraw(void);						// redraws the window on screen

protected:
	void puts(char *str);						// like puts(), but in the window
	void outtext(char *str);					// outputs text, but does no linefeed

	void setcursor(unsigned int newx, unsigned int newy)	// set window-cursor to new position
	{ curpos = newy*insizex+newx; };
	unsigned int wherex()						// returns cursor x-position
	{ return (curpos % insizex); };
	unsigned int wherey()						// returns cursor y-position
	{ return (curpos / insizex); };
	unsigned int getcursor()					// returns absolute cursor position inside buffer
	{ return curpos; };

	void clear();							// clears the window and resets cursor position

	char *wndbuf;							// inner-window text buffer
	unsigned char *colmap;						// inner-window color map
	unsigned char insizex,insizey;				// inner-window sizes

private:
	// flags
	bool visible;							// visibility flag
	bool refreshbb;							// refresh background buffer flag

	// positions, sizes & colors
	unsigned char x,y,sizex,sizey;				// window position and size
	unsigned int curpos;						// cursor position inside window text buffer
	unsigned char color[MAXCOLORS];

	// buffers
	char caption[MAXCAPTION+1];					// window caption
	char *backbuf;							// window background buffer
};

class CTxtWnd: public CWindow
{
public:
	CTxtWnd(unsigned char nsizex = DEFWNDSIZEX, unsigned char nsizey = DEFWNDSIZEY, unsigned char nx = DEFWNDPOSX, unsigned char ny = DEFWNDPOSY);
	~CTxtWnd()
	{ delete [] txtbuf; };

//	void printf(char *format, ...);				// like printf(), but in the window
	void outtext(const char *str);				// outputs text, but does no linefeed
	void puts(const char *str)					// like puts(), but in the window
	{ outtext(str); outtext("\n"); };

	void scroll_set(unsigned int ns)
	{ start = ns * insizex; };
	void scroll_down(unsigned int amount = 1);
	void scroll_up(unsigned int amount = 1);

	void clear();							// clears text buffer

	void redraw();

private:
	unsigned int txtpos,bufsize,start;
	char *txtbuf;
};

class CListWnd: public CWindow
{
public:
	CListWnd(unsigned char nsizex = DEFWNDSIZEX, unsigned char nsizey = DEFWNDSIZEY, unsigned char nx = DEFWNDPOSX, unsigned char ny = DEFWNDPOSY);

	unsigned int additem(char *str);
	void removeitem(unsigned int nr);
	void removeall();
	char *getitem(unsigned int nr);

	void selectitem(unsigned int nr)
	{ selected = nr; };
	void select_next();
	void select_prev();
	unsigned int getselection()
	{ return selected; };

	void scroll_set(unsigned int sc)
	{ start = sc; };
	void scroll_down()
	{ if(start+insizey < numitems) start++; };
	void scroll_up()
	{ if(start) start--; };

	enum Color {colBorder, colIn, colCaption, colSelect, colUnselect};

	void setcolor(Color c, unsigned char v);			// sets window color
	unsigned char getcolor(Color c);				// returns window color

	void redraw();

private:
	char item[MAXITEMS][MAXITEMSTRING];
	bool useitem[MAXITEMS];

	unsigned int selected,start,numitems;
	unsigned char selcol,unselcol;
};

/*
 * txtgfx.c - Textmode Graphics Library, by Simon Peter (dn.tlp@gmx.net)
 */

#include <i86.h>
#include <string.h>

#define SCRSIZEX		80
#define SCRSIZEY		25

static unsigned char curcol=7;
static unsigned int scrpos=0;
static unsigned char *vptr = 0xb8000;	// pointer to textmode vidmem

void clearscreen(unsigned char col)
{
	unsigned int i;

	for(i=0;i<SCRSIZEX*SCRSIZEY;i++) {
		vptr[i*2] = 0;
		vptr[i*2+1] = col;
	}
	scrpos = 0;
}

void setvideomode(unsigned char mode)
{
	union REGS regs;

	regs.h.ah = 0;
	regs.h.al = mode;

	int386(0x10, &regs, &regs);
}

void showcursor(void)
{
	union REGS regs;

	regs.h.ah = 1;
	regs.h.ch = 6;
	regs.h.cl = 7;

	int386(0x10, &regs, &regs);
}

void hidecursor(void)
{
	union REGS regs;

	regs.h.ah = 1;
	regs.w.cx = 0xffff;

	int386(0x10, &regs, &regs);
}

void setcolor(unsigned char newcol)
{
	curcol = newcol;
}

void settextposition(unsigned char ypos, unsigned char xpos)
{
	scrpos = ypos*SCRSIZEX+xpos;
}

void outtext(char *str)
/* writes str directly into video memory */
{
	unsigned int i;

	for(i=0;i<strlen(str);i++) {
		vptr[(scrpos+i)*2] = str[i];
		vptr[(scrpos+i)*2+1] = curcol;
	}
	scrpos += strlen(str);
}

void outchar(char c)
{
	char str[2];

	str[0] = c; str[1] = '\0';
	outtext(str);
}

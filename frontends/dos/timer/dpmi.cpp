// OpenCP Module Player
// copyright (c) '94-'98 Niklas Beisert <nbeisert@physik.tu-muenchen.de>
//
//#ifdef DOS32
// various dpmi call handling routines
//
// revision history: (please note changes here)
//  -nb980510   Niklas Beisert <nbeisert@physik.tu-muenchen.de>
//    -first release
//  -kb980717   Tammo Hinrichs <opencp@gmx.net>
//    -added setWinTitle() to set the title of a Win9x DOS Box window

#include <string.h>
#include <i86.h>
#include "dpmi.h"

void *dosmalloc(unsigned long len, void __far16 *&rmptr, __segment &pmsel)
{
  len=(len+15)>>4;
  REGS r;
  r.w.ax=0x100;
  r.w.bx=len;
  int386(0x31, &r, &r);
  if (r.x.cflag)
    return 0;
  pmsel=r.w.dx;
  rmptr=(void __far16 *)((unsigned long)r.w.ax<<16);
  return (void *)((unsigned long)r.w.ax<<4);
}


void dosfree(__segment pmsel)
{
  REGS r;
  r.w.ax=0x101;
  r.w.dx=pmsel;
  int386(0x31, &r, &r);
}


/*void far *getvect(unsigned char intno)
{
  REGS r;
  SREGS sr;
  r.h.ah=0x35;
  r.h.al=intno;
  sr.ds=sr.es=0;
  int386x(0x21, &r, &r, &sr);
  return MK_FP(sr.es, r.x.ebx);
}

void setvect(unsigned char intno, void far *vect)
{
  REGS r;
  SREGS sr;
  r.h.ah=0x25;
  r.h.al=intno;
  r.x.edx=FP_OFF(vect);
  sr.ds=FP_SEG(vect);
  sr.es=0;
  int386x(0x21, &r, &r, &sr);
} */

void intrrm(unsigned char intno, callrmstruct &regs)
{
  REGS r;
  r.w.ax=0x300;
  r.w.bx=intno;
  r.w.cx=0;
  r.x.edi=(unsigned long)&regs;
  regs.s.ip=regs.s.cs=regs.s.sp=regs.s.ss=0;
  regs.s.flags&=~0x300;
  int386(0x31, &r, &r);
}

void clearcallrm(callrmstruct &r)
{
  memset(&r, 0, sizeof(r));
}

void far *getexvect(unsigned char intno)
{
  REGS r;
  r.w.ax=0x202;
  r.h.bl=intno;
  int386(0x31, &r, &r);
  return MK_FP(r.w.cx, r.x.edx);
}

void setexvect(unsigned char intno, void far *vect)
{
  REGS r;
  r.w.ax=0x203;
  r.h.bl=intno;
  r.w.cx=FP_SEG(vect);
  r.x.edx=FP_OFF(vect);
  int386(0x31, &r, &r);
}


void setwintitle(char *t)
{
  if (!t)
    return;

  void __far16 *rp;
  __segment psel;

  char *strbuf=(char *)dosmalloc(strlen(t)+1,rp,psel);
  strcpy(strbuf,t);

  callrmstruct r;
  r.w.ax = 0x168e;
  r.w.dx = 0;
  r.s.es = ((unsigned long) rp)>>16;
  r.w.di = ((unsigned long) rp) & 0xFFFF;
  intrrm(0x2F,r);

  dosfree(psel);

}

void __far16 *getrmvect(unsigned char intno)
{
  REGS r;
  r.w.ax=0x200;
  r.h.bl=intno;
  int386(0x31, &r, &r);
  return (void __far16 *)((r.w.cx<<16)|r.w.dx);
}

void setrmvect(unsigned char intno, void __far16 *vect)
{
  REGS r;
  r.w.ax=0x201;
  r.h.bl=intno;
  r.w.cx=((unsigned long)vect)>>16;
  r.w.dx=((unsigned long)vect)&0xFFFF;
  int386(0x31, &r, &r);
}

//#endif

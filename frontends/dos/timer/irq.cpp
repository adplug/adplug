// OpenCP Module Player
// copyright (c) '94-'98 Niklas Beisert <nbeisert@physik.tu-muenchen.de>
//
// IRQ handlers
//
// revision history: (please note changes here)
//  -nb980510   Niklas Beisert <nbeisert@physik.tu-muenchen.de>
//    -first release

#include <i86.h>
#include <conio.h>

static unsigned char irqNum;
static unsigned char irqIntNum;
static unsigned char irqPort;
static void far *irqOldInt;
static unsigned char irqOldMask;
static void (*irqRoutine)();
static unsigned char irqPreEOI;
static char __far *stack;
static unsigned long stacksize;
static unsigned char stackused;
static void __far *oldssesp;

void far *getvect(unsigned char intno)
{
  REGS r;
  SREGS sr;
  r.h.ah=0x35;
  r.h.al=intno;
  sr.ds=sr.es=0;
  int386x(0x21, &r, &r, &sr);
  return MK_FP(sr.es, r.x.ebx);
}

void setvect(unsigned char intno, void far * vect)
{
  REGS r;
  SREGS sr;
  r.h.ah=0x25;
  r.h.al=intno;
  r.x.edx=FP_OFF(vect);
  sr.ds=FP_SEG(vect);
  sr.es=0;
  int386x(0x21, &r, &r, &sr);
}

void stackcall(void *);
#pragma aux stackcall parm [eax] = \
  "mov word ptr oldssesp+4,ss" \
  "mov dword ptr oldssesp+0,esp" \
  "lss esp,stack" \
  "sti" \
  "call eax" \
  "cli" \
  "lss esp,oldssesp"

void loades();
#pragma aux loades = "push ds" "pop es"

static void __interrupt irqInt()
{
  loades();

  if (irqPreEOI)
  {
    outp(irqPort, inp(irqPort)|(1<<(irqNum&7)));
    if (irqNum&8)
      outp(0xA0,0x20);
    outp(0x20,0x20);
    if (!stackused)
    {
      stackused++;
      stackcall(irqRoutine);
      stackused--;
    }
    outp(irqPort, inp(irqPort)&~(1<<(irqNum&7)));
  }
  else
  {
    if (!stackused)
    {
      stackused++;
      stackcall(irqRoutine);
      stackused--;
    }
    if (irqNum&8)
      outp(0xA0,0x20);
    outp(0x20,0x20);
  }
}

int irqInit(int inum, void (*routine)(), int pre, int stk)
{
  stacksize=stk;
  stack=new char [stacksize];
  if (!stack)
    return 0;
  stack+=stacksize;

  irqPreEOI=pre;
  inum&=15;
  irqNum=inum;
  irqIntNum=(inum&8)?(inum+0x68):(inum+8);
  irqPort=(inum&8)?0xA1:0x21;

  irqOldInt=getvect(irqIntNum);
  irqOldMask=inp(irqPort)&(1<<(inum&7));

  irqRoutine=routine;
  setvect(irqIntNum, irqInt);
  outp(irqPort, inp(irqPort)&~(1<<(inum&7)));
  return 1;
}

void irqClose()
{
  outp(irqPort, inp(irqPort)|irqOldMask);
  setvect(irqIntNum, irqOldInt);
  delete (char near *)(stack-stacksize);
}

void irqReInit()
{
  outp(irqPort, inp(irqPort)&~(1<<(irqNum&7)));
}
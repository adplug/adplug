// OpenCP Module Player
// copyright (c) '94-'98 Niklas Beisert <nbeisert@physik.tu-muenchen.de>
//
// Systemtimer handlers
//
// revision history: (please note changes here)
//  -nb980510   Niklas Beisert <nbeisert@physik.tu-muenchen.de>
//    -first release
//  -doj990328  Dirk Jagdmann  <doj@cubic.org>
//    -changed interrupt access calls to calls from irq.h
//  -fd990518   Felix Domke  <tmbinc@gmx.net>
//    -added CLD after the tmOldTimer-call.
//     this removed the devwmix*-STRANGEBUG. (finally, i hope)
//  -fd990817   Felix Domke  <tmbinc@gmx.net>
//    -added tmSetSecure/tmReleaseSecure to ensure that timer is only
//     called when not "indos". needed for devpVXD (and some other maybe).

#include <conio.h>
#include <i86.h>
#include "imsrtns.h"
#include "irq.h"
#include "pindos.h"

static void (__far __interrupt *tmOldTimer)();
static void (*tmTimerRoutine)();
static unsigned long tmTimerRate;
static unsigned long tmTicker;
static unsigned long tmIntCount;
static char __far *stack;
static volatile unsigned long stacksize;
static volatile unsigned char stackused;
static void __far *oldssesp;

static volatile float cpuusage;
static volatile char overload;

static int secure;

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


void savefpu();
#pragma aux savefpu = "sub esp,200" "fsave [esp]"

void clearfpu();
#pragma aux clearfpu = "finit"

void restorefpu();
#pragma aux restorefpu = "frstor [esp]" "add esp,200"

void cld();
#pragma aux cld="cld";



static void __interrupt __loadds tmTimerHandler()
{
  savefpu();
  clearfpu();

  loades();

  //outp(0x43,0x34);
  //outp(0x40,tmTimerRate);
  //outp(0x40,tmTimerRate>>8);

  tmTicker+=tmTimerRate;

  tmIntCount+=tmTimerRate;
  if (tmIntCount&0xFFFF0000)
  {
    tmIntCount&=0xFFFF;
    tmOldTimer();
    cld();
  }

  outp(0x20,0x20);

  if (stackused)
  {
    cpuusage=100;
    overload=1;
    restorefpu();
    return;
  }

  stackused++;
  if ((!secure) || (!indosCheck()))
    stackcall(tmTimerRoutine);
  stackused--;

/*  if (indosCheck())
  {
    outp(0x3c8,0);
    outp(0x3c9,0);
    outp(0x3c9,0);
    outp(0x3c9,63);
  } else
  {
    outp(0x3c8,0);
    outp(0x3c9,0);
    outp(0x3c9,0);
    outp(0x3c9,0);
  } */


  if (!overload)
  {
    unsigned long tv=tmTimerRate;
    outp(0x43,0);
    tv-=inp(0x40);
    tv-=inp(0x40)<<8;
    cpuusage=0.1*(100*tv/tmTimerRate)+0.9*cpuusage;
  }
  else
    cpuusage=100;
  overload=0;

  restorefpu();
}

int tmInit(void (*rout)(), int timerval, int stk)
{
  stacksize=stk;
  stack=new char [stacksize];
  if (!stack)
    return 0;
  stack+=stacksize;

  tmTimerRoutine=rout;
  tmIntCount=0;
  tmTicker=-timerval;
  tmTimerRate=timerval;

  tmOldTimer=(void (__far __interrupt *)())getvect(0x08);
  setvect(0x08, tmTimerHandler);

  outp(0x43, 0x34);
  outp(0x40, tmTimerRate);
  outp(0x40, (tmTimerRate>>8));

  cpuusage=0;

  return 1;
}

void tmSetNewRate(int val)
{
  tmTimerRate=val;
  outp(0x43,0x34);
  outp(0x40,tmTimerRate);
  outp(0x40,tmTimerRate>>8);
}

int tmGetTicker()
{
  return tmTicker;
}

void tmSetTicker(int t)
{
  tmTicker+=t-tmGetTicker();
}

int tmGetTimer()
{
  unsigned long tm=tmTimerRate+tmTicker;

//  register unsigned short f=_disableint();
  outp(0x43,0);
  tm-=inp(0x40);
  tm-=inp(0x40)<<8;
//  _restoreint(f);

  return umulshr16(tm, 3600);
}

void tmClose()
{
  setvect(0x08, tmOldTimer);
  outp(0x43, 0x34);
  outp(0x40, 0x00);
  outp(0x40, 0x00);
  delete (char near *)(stack-stacksize);
}

int tmGetCpuUsage()
{
  return (char)cpuusage;
}

void tmSetSecure()
{
  secure++;
}

void tmReleaseSecure()
{
  if (secure)
    secure--;
}

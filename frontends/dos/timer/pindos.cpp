#include <string.h>
#include "dpmi.h"

static unsigned char *indosflag;
static __segment dosmemsel;
static unsigned char *dosmem;

int indosInit()
{
  callrmstruct r;
  r.b.ah=0x34;
  intrrm(0x21, r);
  indosflag=(unsigned char*)((r.s.es<<4)+r.w.bx);

  void __far16 *rmptr;
  dosmem=(unsigned char*)dosmalloc(32*3, rmptr, dosmemsel);
  if (!dosmem)
    return 0;

  memcpy(dosmem, "\x00\x2E\xFE\x06\x00\x00\x9C\x9A\x00\x00\x00\x00\xFB\x51\xB9\x00\x01\xE2\xFE\x59\x2E\xFE\x0E\x00\x00\xCF\x00\x00\x00\x00\x00\x00", 32);
  int i;
  for (i=1; i<3; i++)
    memcpy(dosmem+i*32, "\x00\x9C\x2E\xFE\x06\x00\x00\x9D\x9C\x9A\x00\x00\x00\x00\x55\x9C\x8B\xEC\x8F\x46\x08\x5D\x2E\xFE\x0E\x00\x00\xCF\x00\x00\x00\x00", 32);

  *(void __far16 **)(dosmem+0x08)=getrmvect(0x28);
  setrmvect(0x28, (void __far16 *)((unsigned long)rmptr+0x00000001));
  *(void __far16 **)(dosmem+0x2A)=getrmvect(0x13);
//  setrmvect(0x13, (void __far16 *)((unsigned long)rmptr+0x00020001));
  *(void __far16 **)(dosmem+0x4A)=getrmvect(0x2F);
//  setrmvect(0x2F, (void __far16 *)((unsigned long)rmptr+0x00040001));

  return 1;
}

void indosClose()
{
  setrmvect(0x28, *(void __far16 **)(dosmem+0x08));
  setrmvect(0x13, *(void __far16 **)(dosmem+0x2A));
  setrmvect(0x2F, *(void __far16 **)(dosmem+0x4A));
  dosfree(dosmemsel);
}

int indosCheck()
{
  if (dosmem[0x20]||dosmem[0x40]||(*indosflag>1))
    return 1;

  if (!*indosflag)
    return 0;
  if (dosmem[0x00])
    return 0;
  return 1;
}

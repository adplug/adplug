#ifndef __DPMI_H
#define __DPMI_H

struct callrmstruct;

void *dosmalloc(unsigned long len, void __far16 *&rmptr, __segment &pmsel);
void dosfree(__segment pmsel);
__segment getselectors(void *base, unsigned long len, unsigned short n);
void freeselectors(__segment sel, unsigned short n);

void __far *getvect(unsigned char intno);
void setvect(unsigned char intno, void __far *vect);
void __far16 *getrmvect(unsigned char intno);
void setrmvect(unsigned char intno, void __far16 *vect);
void __far *getpmvect(unsigned char intno);
void setpmvect(unsigned char intno, void __far *vect);
void __far *getexvect(unsigned char intno);
void setexvect(unsigned char intno, void __far *vect);
void *getphysicalmapping(unsigned long addr, unsigned long size);
void freephysicalmapping(void *p);
void intrrm(unsigned char intno, callrmstruct &regs);
void callrm(void __far16 *fn, callrmstruct &regs);
void callrmiret(void __far16 *fn, callrmstruct &regs);
void clearcallrm(callrmstruct &r);

void setwintitle(char *t);

struct callrmstruct
{
union
{
  struct
  {
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long esp;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
  } x;
  struct
  {
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long esp;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
  } d;
  struct
  {
    unsigned short di; unsigned short _1;
    unsigned short si; unsigned short _2;
    unsigned short bp; unsigned short _3;
    unsigned short sp; unsigned short _4;
    unsigned short bx; unsigned short _5;
    unsigned short dx; unsigned short _6;
    unsigned short cx; unsigned short _7;
    unsigned short ax; unsigned short _8;
  } w;
  struct
  {
    unsigned long _1;
    unsigned long _2;
    unsigned long _3;
    unsigned long _4;
    unsigned char bl,bh; unsigned short _5;
    unsigned char dl,dh; unsigned short _6;
    unsigned char cl,ch; unsigned short _7;
    unsigned char al,ah; unsigned short _8;
  } b;
  struct
  {
    unsigned long _1;
    unsigned long _2;
    unsigned long _3;
    unsigned long _4;
    unsigned long _5;
    unsigned long _6;
    unsigned long _7;
    unsigned long _8;
    unsigned short flags;
    unsigned short es;
    unsigned short ds;
    unsigned short fs;
    unsigned short gs;
    unsigned short ip;
    unsigned short cs;
    unsigned short sp;
    unsigned short ss;
  } s;
};
};

void *dpmiMapMemory(long phys, int size);
#pragma aux dpmiMapMemory="mov eax, 0800h", \
                      "push ecx",       \
                      "pop di",         \
                      "pop si",         \
                      "push ebx",       \
                      "pop cx",         \
                      "pop bx",         \
                      "int 31h",        \
                      "push bx",        \
                      "push cx",        \
                      "pop eax",        \
                      "ee: nop" parm [ebx] [ecx] modify [eax edx] value [eax];

int dpmiUnMapMemory(void *mem);
#pragma aux dpmiUnMapMemory="mov eax, 0801h", \
                          "push ecx",       \
                          "pop cx",         \
                          "pop bx",         \
                          "int 31h",        \
                          parm [ebx] modify [eax edx ecx] value [eax];


#endif

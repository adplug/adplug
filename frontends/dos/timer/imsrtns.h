// OpenCP Module Player
// copyright (c) '94-'98 Niklas Beisert <nbeisert@physik.tu-muenchen.de>
//
// DOS4GFIX initialisation handlers
//
// revision history: (please note changes here)
//  -nb980510   Niklas Beisert <nbeisert@physik.tu-muenchen.de>
//    -first release
//  -kb98????   Tammo Hinrichs <opencp@gmx.net>                 (?)
//    -some bugfix(?)
//  -fd981205   Felix Domke <tmbinc@gmx.net>
//    -KB's "bugfix" doesn't work with Watcom106

long imuldiv(long,long,long);
#pragma aux imuldiv parm [eax] [edx] [ebx] value [eax] = "imul edx" "idiv ebx"
unsigned long umuldiv(unsigned long,unsigned long,unsigned long);
#pragma aux umuldiv parm [eax] [edx] [ebx] value [eax] = "mul edx" "div ebx"
long imulshr16(long,long);
#pragma aux imulshr16 parm [eax] [edx] [ebx] value [eax] = "imul edx" "shrd eax,edx,16"
unsigned long umulshr16(unsigned long,unsigned long);
#pragma aux umulshr16 parm [eax] [edx] [ebx] value [eax] = "mul edx" "shrd eax,edx,16"
unsigned long umuldivrnd(unsigned long, unsigned long, unsigned long);
#pragma aux umuldivrnd parm [eax] [edx] [ecx] modify [ebx] = "mul edx" "mov ebx,ecx" "shr ebx,1" "add eax,ebx" "adc edx,0" "div ecx"
void memsetd(void *, long, int);
#pragma aux memsetd parm [edi] [eax] [ecx] = "rep stosd"
void memsetw(void *, int, int);
#pragma aux memsetw parm [edi] [eax] [ecx] = "rep stosw"
void memsetb(void *, int, int);
#pragma aux memsetb parm [edi] [eax] [ecx] = "rep stosb"
void memcpyb(void *, void *, int);
#pragma aux memcpyb parm [edi] [esi] [ecx] = "rep movsb" modify [eax esi edi ecx]

short _disableint();
void _restoreint(short);

#ifdef WATCOM11
#pragma aux _disableint value [ax] modify exact [] = \
                         "pushf" \
                         "pop ax" \
                         "cli"
#pragma aux _restoreint parm [ax] modify exact [] = \
                         "push ax" \
                         "popf"
#else

#pragma aux _disableint value [ax] = "pushf" "pop ax" "cli"
#pragma aux _restoreint parm [ax] = "push ax" "popf"

#endif

/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999, 2000, 2001 Simon Peter, <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * "ADLIBEMU.C" Copyright (c) 1998-2001 Ken Silverman
 *  Released here with permission of the author under the LGPL.
 *  Ken Silverman's official web site: "http://www.advsys.net/ken"
 */
/*
This file is a digital Adlib emulator for OPL2 and possibly OPL3

I intend to support these features in the future:
	- Amplitude and Frequency Vibrato Bits (not hard, but a big speed hit)
	- Global Keyboard Split Number Bit (need to research this one some more)
	- 2nd Adlib chip for OPL3 (simply need to make my cell array bigger)
	- Advanced connection modes of OPL3 (Just need to add more "docell" cases)
	- L/R Stereo bits of OPL3 (Need adlibgetsample to return stereo)

I don't intend to support these since these parts of the adlib chip are useless
	- Anything related to adlib timers&interrupts (Sorry - I always used IRQ0)
	- Composite sine wave mode (CSM) (Supported only on ancient cards)

I'm not sure about a few things in my code:
	- Attack curve.  What function is this anyway?  I chose to use an order-3
		  polynomial to approximate but this doesn't seem right.
	- Attack/Decay/Release constants - my constants may not exact
	- What should ADJUSTSPEED be?
	- Haven't verified that Global Keyboard Split Number Bit works yet
	- Some of the drums don't always sound right.  It's pretty hard to guess
		  the exact waveform of drums when you look at random data which is
		  slightly randomized due to digital ADC recording.
	- Adlib seems to have a lot more treble than my emulator does.  I'm not
		  sure if this is simply unfixable due to the sound blaster's different
		  filtering on FM and digital playback or if it's a serious bug in my
		  code.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "adlibemu.h"

#if !defined(max) && !defined(__cplusplus)
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#if !defined(min) && !defined(__cplusplus)
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef PI
#ifdef M_PI
#define PI M_PI
#else
#define PI 3.141592653589793
#endif
#endif

#define WAVPREC 2048

#define AMPSCALE 8192.0
#define FRQSCALE (49716/512.0)

	//Constants for Ken's Awe32, on a PII-266 (Ken says: Use these for KSM's!)
#define MODFACTOR 4.0      //How much of modulator cell goes into carrier
#define MFBFACTOR 1.0      //How much feedback goes back into modulator
#define ADJUSTSPEED 0.75   //0<=x<=1  Simulate finite rate of change of state

	//Constants for Ken's Awe64G, on a P-133
//#define MODFACTOR 4.25   //How much of modulator cell goes into carrier
//#define MFBFACTOR 0.5    //How much feedback goes back into modulator
//#define ADJUSTSPEED 0.85 //0<=x<=1  Simulate finite rate of change of state

static float frqmul[16] = {.5,1,2,3,4,5,6,7,8,9,10,10,12,12,15,15};
static float kslmul[4] = {0.0,0.5,0.25,1.0};
static unsigned char modulatorbase[9] = {0,1,2,8,9,10,16,17,18};
static unsigned char base2cell[22] = {0,1,2,0,1,2,0,0,3,4,5,3,4,5,0,0,6,7,8,6,7,8};
static unsigned char ksl[8][16];

static signed short *wavtable = 0;
static unsigned long adlib_count = 0;

static _inline void ftol (float f, long *a)
{
	_asm
	{
		mov eax, a
		fld f
		fistp dword ptr [eax]
	}
}

#define ctc ((celltype *)c)      //A rare attempt to make code easier to read!
void docell4 (void *c, float modulator) { }
void docell3 (void *c, float modulator)
{
	long i;

	ftol(ctc->t+modulator,&i);
	ctc->t += ctc->tinc;
	ctc->val += (ctc->amp*ctc->vol*((float)ctc->waveform[i&ctc->wavemask])-ctc->val)*ADJUSTSPEED;
}
void docell2 (void *c, float modulator)
{
	long i;

	ftol(ctc->t+modulator,&i);

	if (*(long *)&ctc->amp <= 0x37800000)
	{
		ctc->amp = 0;
		ctc->cellfunc = docell4;
	}
	ctc->amp *= ctc->releasemul;

	ctc->t += ctc->tinc;
	ctc->val += (ctc->amp*ctc->vol*((float)ctc->waveform[i&ctc->wavemask])-ctc->val)*ADJUSTSPEED;
}
void docell1 (void *c, float modulator)
{
	long i;

	ftol(ctc->t+modulator,&i);

	if ((*(long *)&ctc->amp) <= (*(long *)&ctc->sustain))
	{
		if (ctc->flags&32)
		{
			ctc->amp = ctc->sustain;
			ctc->cellfunc = docell3;
		}
		else
			ctc->cellfunc = docell2;
	}
	else
		ctc->amp *= ctc->decaymul;

	ctc->t += ctc->tinc;
	ctc->val += (ctc->amp*ctc->vol*((float)ctc->waveform[i&ctc->wavemask])-ctc->val)*ADJUSTSPEED;
}
void docell0 (void *c, float modulator)
{
	long i;

	ftol(ctc->t+modulator,&i);

	ctc->amp = ((ctc->a3*ctc->amp + ctc->a2)*ctc->amp + ctc->a1)*ctc->amp + ctc->a0;
	if ((*(long *)&ctc->amp) > 0x3f800000)
	{
		ctc->amp = 1;
		ctc->cellfunc = docell1;
	}

	ctc->t += ctc->tinc;
	ctc->val += (ctc->amp*ctc->vol*((float)ctc->waveform[i&ctc->wavemask])-ctc->val)*ADJUSTSPEED;
}


static long waveform[8] = {WAVPREC,WAVPREC>>1,WAVPREC,(WAVPREC*3)>>2,0,0,(WAVPREC*5)>>2,WAVPREC<<1};
static long wavemask[8] = {WAVPREC-1,WAVPREC-1,(WAVPREC>>1)-1,(WAVPREC>>1)-1,WAVPREC-1,((WAVPREC*3)>>2)-1,WAVPREC>>1,WAVPREC-1};
static long wavestart[8] = {0,WAVPREC>>1,0,WAVPREC>>2,0,0,0,WAVPREC>>3};
static float attackconst[4] = {1/2.82624,1/2.25280,1/1.88416,1/1.59744};
static float decrelconst[4] = {1/39.28064,1/31.41608,1/26.17344,1/22.44608};
void cellon(ADLIB_STATE *adlib, long i, long j, celltype *c, unsigned char iscarrier)
{
	long frn, oct, toff;
	float f;

	frn = ((((long)adlib->reg[i+0xb0])&3)<<8) + (long)adlib->reg[i+0xa0];
	oct = ((((long)adlib->reg[i+0xb0])>>2)&7);
	toff = (oct<<1) + ((frn>>9)&((frn>>8)|((adlib->reg[8]>>6)&1^1)));
	if (!(adlib->reg[j+0x20]&16)) toff >>= 2;

	f = pow(2.0,(adlib->reg[j+0x60]>>4)+(toff>>2)-1)*attackconst[toff&3]*adlib->recipsamp;
	c->a0 = .0377*f; c->a1 = 10.73*f+1; c->a2 = -17.57*f; c->a3 = 7.42*f;
	f = -7.4493*decrelconst[toff&3]*adlib->recipsamp;
	c->decaymul = pow(2.0,f*pow(2.0,(adlib->reg[j+0x60]&15)+(toff>>2)));
	c->releasemul = pow(2.0,f*pow(2.0,(adlib->reg[j+0x80]&15)+(toff>>2)));
	c->wavemask = wavemask[adlib->reg[j+0xe0]&7];
	if (!(adlib->reg[1]&0x20)) c->waveform = &wavtable[WAVPREC];
	else c->waveform = &wavtable[waveform[adlib->reg[j+0xe0]&7]];
	c->t = wavestart[adlib->reg[j+0xe0]&7];
	c->flags = adlib->reg[j+0x20];
	c->cellfunc = docell0;
	c->tinc = (float)(frn<<oct)*adlib->nfrqmul[adlib->reg[j+0x20]&15];
	c->vol = pow(2.0,((float)(adlib->reg[j+0x40]&63) +
				(float)kslmul[adlib->reg[j+0x40]>>6]*ksl[oct][frn>>6]) * -.125 - 14);
	c->sustain = pow(2.0,(float)(adlib->reg[j+0x80]>>4) * -.5);
	if (!iscarrier) c->amp = 0;
	c->mfb = pow(2.0,((adlib->reg[i+0xc0]>>1)&7)+5)*(WAVPREC/2048.0)*MFBFACTOR;
	if (!(adlib->reg[i+0xc0]&14)) c->mfb = 0;
	c->val = 0;
}

	//This function (and bug fix) written by Chris Moeller
void cellfreq(ADLIB_STATE *adlib, long i, long j, celltype *c)
{
	signed long frn, oct, toff;
	float f;
	frn = ((((signed long)adlib->reg[i+0xb0])&3)<<8) + ((signed long)adlib->reg[i+0xa0]&255);
	oct = ((((signed long)adlib->reg[i+0xb0])>>2)&7);
	toff = (oct<<1) + ((frn>>9)&((frn>>8)|((adlib->reg[8]>>6)&1^1)));
	if (!(adlib->reg[j+0x20]&16)) toff >>= 2;

	c->tinc = (float)(frn<<oct)*adlib->nfrqmul[adlib->reg[j+0x20]&15];
	c->vol = pow(2.0,((float)(adlib->reg[j+0x40]&63) +
				(float)kslmul[adlib->reg[j+0x40]>>6]*ksl[oct][frn>>6]) * -.125 - 14);

}

ADLIB_STATE *adlibinit(long dasamplerate, long danumspeakers, long dabytespersample)
{
	signed long i, j, frn, oct;

	ADLIB_STATE *adlib;

	adlib = malloc(sizeof(ADLIB_STATE));

	if (!adlib) return 0;

	memset((void *)adlib, 0, sizeof(ADLIB_STATE)); // is this really necessary?

	adlib->numspeakers = danumspeakers;
	adlib->bytespersample = dabytespersample;

	adlib->recipsamp = 1.0 / (float)dasamplerate;

	for(i=15;i>=0;i--) adlib->nfrqmul[i] = frqmul[i]*adlib->recipsamp*FRQSCALE*(WAVPREC/2048.0);

	for (i=0;i<9;i++)
	{
		adlib->lvol[i]=adlib->rvol[i]=1.0;
		adlib->lplc[i]=adlib->rplc[i]=0;
	}

	for(i=0;i<MAXCELLS;i++)
	{
		adlib->cell[i].cellfunc = docell4;
		adlib->cell[i].waveform = &wavtable[WAVPREC];
	}

	adlib_count++;

	if (!wavtable)
	{
		wavtable = (signed short *)malloc(sizeof(signed short)*WAVPREC*3);

		for(i=0;i<(WAVPREC>>1);i++)
		{
			wavtable[i] =
			wavtable[(i<<1)  +WAVPREC] = (signed short)(16384*sin((float)((i<<1)  )*PI*2/WAVPREC));
			wavtable[(i<<1)+1+WAVPREC] = (signed short)(16384*sin((float)((i<<1)+1)*PI*2/WAVPREC));
		}
		for(i=0;i<(WAVPREC>>3);i++)
		{
			wavtable[i+(WAVPREC<<1)] = wavtable[i+(WAVPREC>>3)]-16384;
			wavtable[i+((WAVPREC*17)>>3)] = wavtable[i+(WAVPREC>>2)]+16384;
		}

			//[table in book]*8/3
		ksl[7][0] = 0; ksl[7][1] = 24; ksl[7][2] = 32; ksl[7][3] = 37;
		ksl[7][4] = 40; ksl[7][5] = 43; ksl[7][6] = 45; ksl[7][7] = 47;
		ksl[7][8] = 48; for(i=9;i<16;i++) ksl[7][i] = i+41;
		for(j=6;j>=0;j--)
			for(i=0;i<16;i++)
			{
				oct = (long)ksl[j+1][i]-8; if (oct < 0) oct = 0;
				ksl[j][i] = (unsigned char)oct;
			}
	}

	return adlib;
}

void adlibwrite (ADLIB_STATE *adlib, long i, long v)
{
	unsigned char tmp = adlib->reg[i];
	adlib->reg[i] = (unsigned char)v;

	if (adlib->UpdateHandler) adlib->UpdateHandler(adlib->UpdateParam, 0);

	if (i == 0xbd)
	{
		if ((v&16) > (tmp&16)) //BassDrum
		{
			cellon(adlib,6,16,&adlib->cell[6],0);
			cellon(adlib,6,19,&adlib->cell[15],1);
			adlib->cell[15].vol *= 2;
		}
		else if ((v&16) < (tmp&16))
			adlib->cell[6].cellfunc = adlib->cell[15].cellfunc = docell2;

		if ((v&8) > (tmp&8)) //Snare
		{
			cellon(adlib,16,20,&adlib->cell[16],0);
			adlib->cell[16].tinc *= 2*(adlib->nfrqmul[adlib->reg[17+0x20]&15] / adlib->nfrqmul[adlib->reg[20+0x20]&15]);
			if (((adlib->reg[20+0xe0]&7) >= 3) && ((adlib->reg[20+0xe0]&7) <= 5)) adlib->cell[16].vol = 0;
			adlib->cell[16].vol *= 2;
		}
		else if ((v&8) < (tmp&8))
			adlib->cell[16].cellfunc = docell2;

		if ((v&4) > (tmp&4)) //TomTom
		{
			cellon(adlib,8,18,&adlib->cell[8],0);
			adlib->cell[8].vol *= 2;
		}
		else if ((v&4) < (tmp&4))
			adlib->cell[8].cellfunc = docell2;

		if ((v&2) > (tmp&2)) //Cymbal
		{
			cellon(adlib,17,21,&adlib->cell[17],0);

			adlib->cell[17].wavemask = wavemask[5];
			adlib->cell[17].waveform = &wavtable[waveform[5]];
			adlib->cell[17].tinc *= 16; adlib->cell[17].vol *= 2;

			//adlib->cell[17].waveform = &wavtable[WAVPREC]; adlib->cell[17].wavemask = 0;
			//if (((adlib->reg[21+0xe0]&7) == 0) || ((adlib->reg[21+0xe0]&7) == 6))
			//   adlib->cell[17].waveform = &wavtable[(WAVPREC*7)>>2];
			//if (((adlib->reg[21+0xe0]&7) == 2) || ((adlib->reg[21+0xe0]&7) == 3))
			//   adlib->cell[17].waveform = &wavtable[(WAVPREC*5)>>2];
		}
		else if ((v&2) < (tmp&2))
			adlib->cell[17].cellfunc = docell2;

		if ((v&1) > (tmp&1)) //Hihat
		{
			cellon(adlib,7,17,&adlib->cell[7],0);
			if (((adlib->reg[17+0xe0]&7) == 1) || ((adlib->reg[17+0xe0]&7) == 4) ||
				 ((adlib->reg[17+0xe0]&7) == 5) || ((adlib->reg[17+0xe0]&7) == 7)) adlib->cell[7].vol = 0;
			if ((adlib->reg[17+0xe0]&7) == 6) { adlib->cell[7].wavemask = 0; adlib->cell[7].waveform = &wavtable[(WAVPREC*7)>>2]; }
		}
		else if ((v&1) < (tmp&1))
			adlib->cell[7].cellfunc = docell2;

	}
	else if (((unsigned)(i-0x40) < (unsigned)22) && ((i&7) < 6))
	{
		if ((i&7) < 3) //Modulator
			cellfreq(adlib,base2cell[i-0x40],i-0x40,&adlib->cell[base2cell[i-0x40]]);
		else           //Carrier
			cellfreq(adlib,base2cell[i-0x40],i-0x40,&adlib->cell[base2cell[i-0x40]+9]);
	}
	else if ((unsigned)(i-0xa0) < (unsigned)9)
	{
		cellfreq(adlib,i-0xa0,modulatorbase[i-0xa0],&adlib->cell[i-0xa0]);
		cellfreq(adlib,i-0xa0,modulatorbase[i-0xa0]+3,&adlib->cell[i-0xa0+9]);
	}
	else if ((unsigned)(i-0xb0) < (unsigned)9)
	{
		if ((v&32) > (tmp&32))
		{
			cellon(adlib,i-0xb0,modulatorbase[i-0xb0],&adlib->cell[i-0xb0],0);
			cellon(adlib,i-0xb0,modulatorbase[i-0xb0]+3,&adlib->cell[i-0xb0+9],1);
		}
		else if ((v&32) < (tmp&32))
			adlib->cell[i-0xb0].cellfunc = adlib->cell[i-0xb0+9].cellfunc = docell2;
		else
		{
			cellfreq(adlib,i-0xb0,modulatorbase[i-0xb0],&adlib->cell[i-0xb0]);
			cellfreq(adlib,i-0xb0,modulatorbase[i-0xb0]+3,&adlib->cell[i-0xb0+9]);
		}
	}

	//outdata(i,v);
}

static long fpuasm;
static float fakeadd = 8388608.0+128.0;
static _inline void clipit8 (float f, long a)
{
	_asm
	{
		mov edi, a
		fld dword ptr f
		fadd dword ptr fakeadd
		fstp dword ptr fpuasm
		mov eax, fpuasm
		test eax, 0x007fff00
		jz short skipit
		shr eax, 16
		xor eax, -1
skipit: mov byte ptr [edi], al
	}
}

static _inline void clipit16 (float f, long a)
{
	_asm
	{
		mov eax, a
		fld dword ptr f
		fist word ptr [eax]
		cmp word ptr [eax], 0x8000
		jne short skipit2
		fst dword ptr [fpuasm]
		cmp fpuasm, 0x80000000
		sbb word ptr [eax], 0
skipit2: fstp st
	}
}

void adlibgetsample(ADLIB_STATE *adlib, void *sndptr, long numsamples)
{
	long i, j, k = 0, ns, endsamples, rptrs;
	celltype *cptr;
	float f;

	if (adlib->bytespersample == 1) f = AMPSCALE/256.0; else f = AMPSCALE;

	if (adlib->numspeakers == 1)
	{
		adlib->nlvol[0] = adlib->lvol[0]*f;
		for(i=0;i<9;i++) adlib->rptr[i] = &adlib->rbuf[0][0];
		rptrs = 1;
	}
	else
	{
		rptrs = 0;
		for(i=0;i<9;i++)
		{
			if ((!i) || (adlib->lvol[i] != adlib->lvol[i-1]) || (adlib->rvol[i] != adlib->rvol[i-1]) ||
							(adlib->lplc[i] != adlib->lplc[i-1]) || (adlib->rplc[i] != adlib->rplc[i-1]))
			{
				adlib->nlvol[rptrs] = adlib->lvol[i]*f;
				adlib->nrvol[rptrs] = adlib->rvol[i]*f;
				adlib->nlplc[rptrs] = adlib->rend-min(max(adlib->lplc[i],0),FIFOSIZ);
				adlib->nrplc[rptrs] = adlib->rend-min(max(adlib->rplc[i],0),FIFOSIZ);
				rptrs++;
			}
			adlib->rptr[i] = &adlib->rbuf[rptrs-1][0];
		}
	}


	//CPU time used to be somewhat less when emulator was only mono!
	//   Because of no delay fifos!

	for(ns=0;ns<numsamples;ns+=endsamples)
	{
		endsamples = min(FIFOSIZ*2-adlib->rend,FIFOSIZ);
		endsamples = min(endsamples,numsamples-ns);

		for(i=0;i<9;i++)
			adlib->nrptr[i] = &adlib->rptr[i][adlib->rend];
		for(i=0;i<rptrs;i++)
			memset((void *)&adlib->rbuf[i][adlib->rend],0,endsamples*sizeof(float));

		if (adlib->reg[0xbd]&0x20)
		{
				//BassDrum (j=6)
			if (adlib->cell[15].cellfunc != docell4)
			{
				if (adlib->reg[0xc6]&1)
				{
					for(i=0;i<endsamples;i++)
					{
						(adlib->cell[15].cellfunc)((void *)&adlib->cell[15],0.0);
						adlib->nrptr[6][i] += adlib->cell[15].val;
					}
				}
				else
				{
					for(i=0;i<endsamples;i++)
					{
						(adlib->cell[6].cellfunc)((void *)&adlib->cell[6],adlib->cell[6].val*adlib->cell[6].mfb);
						(adlib->cell[15].cellfunc)((void *)&adlib->cell[15],adlib->cell[6].val*WAVPREC*MODFACTOR);
						adlib->nrptr[6][i] += adlib->cell[15].val;
					}
				}
			}

				//Snare/Hihat (j=7), Cymbal/TomTom (j=8)
			if ((adlib->cell[7].cellfunc != docell4) || (adlib->cell[8].cellfunc != docell4) || (adlib->cell[16].cellfunc != docell4) || (adlib->cell[17].cellfunc != docell4))
			{
				for(i=0;i<endsamples;i++)
				{
					k = k*1664525+1013904223;
					(adlib->cell[16].cellfunc)((void *)&adlib->cell[16],k&((WAVPREC>>1)-1)); //Snare
					(adlib->cell[7].cellfunc)((void *)&adlib->cell[7],k&(WAVPREC-1));       //Hihat
					(adlib->cell[17].cellfunc)((void *)&adlib->cell[17],k&((WAVPREC>>3)-1)); //Cymbal
					(adlib->cell[8].cellfunc)((void *)&adlib->cell[8],0.0);                 //TomTom
					adlib->nrptr[7][i] += adlib->cell[7].val + adlib->cell[16].val;
					adlib->nrptr[8][i] += adlib->cell[8].val + adlib->cell[17].val;
				}
			}
			j=6-1;
		}
		else j=9-1;

		for(;j>=0;j--)
		{
			cptr = &adlib->cell[j]; k = j;
			if (adlib->reg[0xc0+k]&1)
			{
				if ((cptr[9].cellfunc == docell4) && (cptr->cellfunc == docell4)) continue;
				for(i=0;i<endsamples;i++)
				{
					(cptr->cellfunc)((void *)cptr,cptr->val*cptr->mfb);
					(cptr->cellfunc)((void *)&cptr[9],0);
					adlib->nrptr[j][i] += cptr[9].val + cptr->val;
				}
			}
			else
			{
				if (cptr[9].cellfunc == docell4) continue;
				for(i=0;i<endsamples;i++)
				{
					(cptr->cellfunc)((void *)cptr,cptr->val*cptr->mfb);
					(cptr[9].cellfunc)((void *)&cptr[9],cptr->val*WAVPREC*MODFACTOR);
					adlib->nrptr[j][i] += cptr[9].val;
				}
			}
		}

		if (adlib->numspeakers == 1)
		{
			if (adlib->bytespersample == 1)
			{
				for(i=endsamples-1;i>=0;i--)
					clipit8(adlib->nrptr[0][i]*adlib->nlvol[0],i+(long)sndptr);
			}
			else
			{
				for(i=endsamples-1;i>=0;i--)
					clipit16(adlib->nrptr[0][i]*adlib->nlvol[0],i+i+(long)sndptr);
			}
		}
		else
		{
			memset((void *)adlib->snd,0,endsamples*sizeof(float)*2);
			for(j=0;j<rptrs;j++)
			{
				for(i=0;i<endsamples;i++)
				{
					adlib->snd[(i<<1)  ] += adlib->rbuf[j][(adlib->nlplc[j]+i)&(FIFOSIZ*2-1)]*adlib->nlvol[j];
					adlib->snd[(i<<1)+1] += adlib->rbuf[j][(adlib->nrplc[j]+i)&(FIFOSIZ*2-1)]*adlib->nrvol[j];
				}
				adlib->nlplc[j] += endsamples;
				adlib->nrplc[j] += endsamples;
			}

			if (adlib->bytespersample == 1)
			{
				for(i=(endsamples<<1)-1;i>=0;i--)
					clipit8(adlib->snd[i],i+((long)sndptr));
			}
			else
			{
				for(i=(endsamples<<1)-1;i>=0;i--)
					clipit16(adlib->snd[i],i+i+((long)sndptr));
			}
		}

		sndptr = (void *)(((long)sndptr)+(adlib->numspeakers*adlib->bytespersample*endsamples));
		adlib->rend = ((adlib->rend+endsamples)&(FIFOSIZ*2-1));
	}
}

void adlibreset(ADLIB_STATE *adlib)
{
	int i;

	memset((void *)adlib->reg,0,sizeof(adlib->reg));
	memset((void *)adlib->cell,0,sizeof(celltype)*MAXCELLS);
	memset((void *)adlib->rbuf,0,sizeof(adlib->rbuf));
	adlib->rend = 0;

	for(i=0;i<MAXCELLS;i++)
	{
		adlib->cell[i].cellfunc = docell4;
		adlib->cell[i].waveform = &wavtable[WAVPREC];
	}
}

void adlibshutdown(ADLIB_STATE *adlib)
{
	if (!adlib || !adlib_count) return;

	adlib_count--;

	if (wavtable && !adlib_count)
	{
		free(wavtable);
		wavtable = 0;
	}

	free(adlib);
}

void adlibsetupdatehandler(ADLIB_STATE *adlib, ADLIB_UPDATEHANDLER UpdateHandler, int param)
{
	adlib->UpdateHandler = UpdateHandler;
	adlib->UpdateParam = param;
}

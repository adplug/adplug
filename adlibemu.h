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
 * Header file for Ken Silverman's "adlibemu.c" Adlib emulator.
 * Released here with permission of the author under the LGPL.
 * "ADLIBEMU.C" Copyright (c) 1998-2001 Ken Silverman
 */

#ifndef __ADLIBEMU_H__
#define __ADLIBEMU_H__

#define MAXCELLS 18
#define FIFOSIZ 256

typedef void (*ADLIB_UPDATEHANDLER)(int param,int min_interval_us);

typedef struct
{
	float val, t, tinc, vol, sustain, amp, mfb;
	float a0, a1, a2, a3, decaymul, releasemul;
	short *waveform;
	long wavemask;
	void (*cellfunc)(void *, float);
	unsigned char flags, dum0, dum1, dum2;
} celltype;

typedef struct
{
	long numspeakers, bytespersample;
	float recipsamp;
	float nfrqmul[16];
	celltype cell[MAXCELLS];
	unsigned char reg[256];

	float lvol[9];  //Volume multiplier on left speaker
	float rvol[9];  //Volume multiplier on right speaker
	long lplc[9];   //Samples to delay on left speaker
	long rplc[9];   //Samples to delay on right speaker

	long nlvol[9];
	long nrvol[9];
	long nlplc[9];
	long nrplc[9];
	long rend;

	float *rptr[9];
	float *nrptr[9];
	float rbuf[9][FIFOSIZ*2];
	float snd[FIFOSIZ*2];

	ADLIB_UPDATEHANDLER UpdateHandler;
	int UpdateParam;

} ADLIB_STATE;

ADLIB_STATE *adlibinit(long dasamplerate, long danumspeakers, long dabytespersample);
void adlibwrite(ADLIB_STATE *adlib, long i, long v);
void adlibgetsample(ADLIB_STATE *adlib, void *sndptr, long numsamples);
void adlibreset(ADLIB_STATE *adlib);
void adlibshutdown(ADLIB_STATE *adlib);
void adlibsetupdatehandler(ADLIB_STATE *adlib, ADLIB_UPDATEHANDLER UpdateHandler, int param);

#endif

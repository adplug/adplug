/*
 *  Copyright (C) 2002-2008  The DOSBox Team
 *  OPL2/OPL3 emulation library
 *
 *  Based on ADLIBEMU.C, an AdLib emulation library by Ken Silverman
 *  Copyright (C) 1998-2001 Ken Silverman
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 * 
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 * 
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/*
This file is a digital Adlib emulator for OPL2 and OPL3
Ken Silverman's official web site: "http://www.advsys.net/ken"

I'm not sure about a few things in my code:
- Attack curve.  What function is this anyway?  I chose to use an order-3
  polynomial to approximate but this doesn't seem right.
- Attack/Decay/Release constants - my constants may not be exact
- Some of the drums don't always sound right.  It's pretty hard to guess
  the exact waveform of drums when you look at random data which is
  slightly randomized due to digital ADC recording.
*/


/*  Changes/additions against original code (by The DOSBox Team):

Features added:
- Timer handling/status word support
- DualOPL2 mode (second OPL2 chip)
- OPL3 mode (32 channels, 4-operator modes, stereo panning, 8 waveforms etc.)
- Allow attackrate/decayrate/releaserate to be always switchable
- Allow waveform type/feedback/keepsustain to be always switchable
- LFO effects (tremolo/vibrato)
- Attempt to imitate step-like envelope function (needed especially when
  slow-fading operator is used as modulator)

Fixes:
- Force decay/release level to stay when decay/release are off
- Force attack rate to zero when attack is off
- Force sustain level to zero for maximal value of sustainlevel
- Fixed mode transitions
- Let modulator progress even if carrier is off
- Correct keysplit handling
- Feedback uses average of last two samples as input
- Additive mode corrected
- Wave table precision corrected

Other changes:
- Removed (almost all) dependencies on 32bit float layout
- Code restructuring/cleanup, added comments to improve readability
- Put everything into a class for an easier multi-OPL implementation

Bugs/missing features:
- Attack/decay/release curve functions are not totally correct
- Step-like effects of the envelope not fully correct
- Percussions are just guesses (CH7+CH8), unknown how good they are
- When exactly does switching the OPL2-compatibility flag (adlibreg[0x105]&1)
  have an effect? (seems not-immediate)
- Optimizations

*/


#include <math.h>
#include <string.h>
#include "woodyopl.h"


static fltype recipsamp;	// inverse of sampling rate
static Bit16s wavtable[WAVPREC*3];	// wave form table

// vibrato/tremolo tables
static Bit32s vib_table[VIBTAB_SIZE];
static fltype trem_table[TREMTAB_SIZE*2];


static fltype val_const[FIFOSIZE];

// vibrato value tables (used per-cell)
static fltype vibval_var1[FIFOSIZE];
static fltype vibval_var2[FIFOSIZE];
static fltype vibval_var3[FIFOSIZE];
static fltype vibval_var4[FIFOSIZE];

// vibrato/trmolo value table pointers
static fltype *vibval1, *vibval2, *vibval3, *vibval4;
static fltype *tremval1, *tremval2, *tremval3, *tremval4;


// key scale level lookup table
static const fltype kslmul[4] = {
	0.0, 0.5, 0.25, 1.0		// -> 0, 3, 1.5, 6 dB/oct
};

// frequency multiplicator lookup table
static const fltype frqmul[16] = {
	0.5,1,2,3,4,5,6,7,8,9,10,10,12,12,15,15
};
// calculated frequency multiplication values (depend on sampling rate)
static fltype nfrqmul[16];

// key scale levels
static Bit8u ksl[8][16];

// map a channel number to the register offset of the modulator
static const Bit8u modulatorbase[9]	= {
	0,1,2,
	8,9,10,
	16,17,18
};

// map a register base to a modulator cell number
static const Bit8u regbase2modcell[22] = {
	0,1,2,0,1,2,0,0,3,4,5,3,4,5,0,0,6,7,8,6,7,8
};


OPLChipClass* oplchip[2];


// start of the waveform
static Bitu waveform[8] = {
	WAVPREC,
	WAVPREC>>1,
	WAVPREC,
	(WAVPREC*3)>>2,
	0,
	0,
	(WAVPREC*5)>>2,
	WAVPREC<<1
};

// length of the waveform as mask
static Bitu wavemask[8] = {
	WAVPREC-1,
	WAVPREC-1,
	(WAVPREC>>1)-1,
	(WAVPREC>>1)-1,
	WAVPREC-1,
	((WAVPREC*3)>>2)-1,
	WAVPREC>>1,
	WAVPREC-1
};

// where the first entry resides
static fltype wavestart[8] = {
	(fltype)(0),
	(fltype)(WAVPREC>>1),
	(fltype)(0),
	(fltype)(WAVPREC>>2),
	(fltype)(0),
	(fltype)(0),
	(fltype)(0),
	(fltype)(WAVPREC>>3)
};

// envelope generator function constants
static fltype attackconst[4] = {1/2.82624,1/2.25280,1/1.88416,1/1.59744};
static fltype decrelconst[4] = {1/39.28064,1/31.41608,1/26.17344,1/22.44608};


OPLChipClass::OPLChipClass(Bitu cnum) {
	// which OPL chip are we (needed for timers)
	chip_num = cnum;
}


// no action, cell is off
void processcell_off(celltype* /*ctc*/, fltype /*modulator*/, fltype /*vib*/, fltype /*trem*/) {
}

// output level is sustained, mode changes only when cell is turned off (->release)
// or when the keep-sustained bit is turned off (->sustain_nokeep)
void processcell_sustain(celltype* ctc, fltype modulator, fltype vib, fltype trem) {
	Bitu i = (Bitu)(ctc->t+modulator);
	ctc->t += (ctc->tinc*vib);		// advance waveform time
	ctc->lastval = ctc->val;

	ctc->generator_pos += generator_add;
	Bits num_steps_add = (Bits)ctc->generator_pos;	// number of (standardized) samples
	for (Bits ct=0; ct<num_steps_add; ct++) {
		ctc->cur_env_step++;
	}
	ctc->generator_pos -= num_steps_add;

	ctc->val = ctc->amp*ctc->vol*((fltype)ctc->cur_wform[i&ctc->cur_wmask])*trem;
}

// cell in release mode, if output level reaches zero the cell is turned off
void processcell_release(celltype* ctc, fltype modulator, fltype vib, fltype trem) {
	// ??? boundary?
	if (ctc->amp > 0.00000001) {
		// release phase
		ctc->amp *= ctc->releasemul;
	} else {
		// release phase finished, turn off this cell
		ctc->cf_sel = CF_TYPE_OFF;
		ctc->amp = 0.0;
		ctc->step_amp = 0.0;
	}

	Bitu i = (Bitu)(ctc->t+modulator);
	ctc->t += (ctc->tinc*vib);		// advance waveform time
	ctc->lastval = ctc->val;

	ctc->generator_pos += generator_add;
	Bits num_steps_add = (Bits)ctc->generator_pos;	// number of (standardized) samples
	for (Bits ct=0; ct<num_steps_add; ct++) {
		ctc->cur_env_step++;						// sample counter
		if ((ctc->cur_env_step&ctc->env_step_r)==0) ctc->step_amp = ctc->amp;
	}
	ctc->generator_pos -= num_steps_add;

	ctc->val = ctc->step_amp*ctc->vol*((fltype)ctc->cur_wform[i&ctc->cur_wmask])*trem;
//	ctc->val = ctc->amp*ctc->vol*((fltype)ctc->cur_wform[i&ctc->cur_wmask])*trem;
}

// cell in decay mode, if sustain level is reached the output level is
// either kept (sustain level keep enabled) or the celll is switched
// into release mode
void processcell_decay(celltype* ctc, fltype modulator, fltype vib, fltype trem) {
	if (ctc->amp > ctc->sustain_level) {
		// decay phase
		ctc->amp *= ctc->decaymul;
	} else {
		// decay phase finished, sustain level reached
		if (ctc->sus_keep) {
			// keep sustain level (until key off)
			ctc->cf_sel = CF_TYPE_SUS;
			ctc->amp = ctc->sustain_level;
			ctc->step_amp = ctc->amp;
		} else {
			// next: release phase
			ctc->cf_sel = CF_TYPE_REL;
			ctc->step_amp = ctc->amp;
		}
	}

	Bitu i = (Bitu)(ctc->t+modulator);
	ctc->t += (ctc->tinc*vib);		// advance waveform time
	ctc->lastval = ctc->val;

	ctc->generator_pos += generator_add;
	Bits num_steps_add = (Bits)ctc->generator_pos;	// number of (standardized) samples
	for (Bits ct=0; ct<num_steps_add; ct++) {
		ctc->cur_env_step++;
		if ((ctc->cur_env_step&ctc->env_step_d)==0) ctc->step_amp = ctc->amp;
	}
	ctc->generator_pos -= num_steps_add;

	ctc->val = ctc->step_amp*ctc->vol*((fltype)ctc->cur_wform[i&ctc->cur_wmask])*trem;
//	ctc->val = ctc->amp*ctc->vol*((fltype)ctc->cur_wform[i&ctc->cur_wmask])*trem;
}

// cell in attack mode, if full output level is reached, the cell is
// switched into decay mode
void processcell_attack(celltype* ctc, fltype modulator, fltype vib, fltype trem) {
	ctc->amp = ((ctc->a3*ctc->amp + ctc->a2)*ctc->amp + ctc->a1)*ctc->amp + ctc->a0;
	if (ctc->amp > 1.0) {
		// attack phase finished, next: decay
		ctc->cf_sel = CF_TYPE_DEC;
		ctc->amp = 1.0;
		ctc->step_amp = 1.0;
	}

	Bitu i = (Bitu)(ctc->t+modulator);
	ctc->t += (ctc->tinc*vib);		// advance waveform time
	ctc->lastval = ctc->val;

	ctc->generator_pos += generator_add;
	Bits num_steps_add = (Bits)ctc->generator_pos;			// determine number of std samples that have passed
	for (Bits ct=0; ct<num_steps_add; ct++) {
		ctc->cur_env_step++;	// next sample
		if ((ctc->cur_env_step&ctc->env_step_a)==0) {		// check if next step already reached
			ctc->step_skip_pos <<= 1;
			if (ctc->step_skip_pos==0) ctc->step_skip_pos = 1;
			if (ctc->step_skip_pos&ctc->env_step_skip_a)	// check if required to skip next step
				ctc->step_amp = ctc->amp;
		}
	}
	ctc->generator_pos -= num_steps_add;

	ctc->val = ctc->step_amp*ctc->vol*((fltype)ctc->cur_wform[i&ctc->cur_wmask])*trem;
//	ctc->val = ctc->amp*ctc->vol*((fltype)ctc->cur_wform[i&ctc->cur_wmask])*trem;
}


typedef void (*cftype_fptr)(celltype*, fltype, fltype, fltype);

cftype_fptr cfuncs[6] = {
	processcell_attack,
	processcell_decay,
	processcell_release,
	processcell_sustain,	// sustain phase (keeping level)
	processcell_release,	// sustain_nokeep phase (release-style)
	processcell_off
};

void OPLChipClass::change_attackrate(Bitu regbase, celltype *c) {
	Bits attackrate = adlibreg[ARC_ATTR_DECR+regbase]>>4;
	if (attackrate) {
		fltype f = (fltype)(pow(FL2,(fltype)attackrate+(c->toff>>2)-1)*attackconst[c->toff&3]*recipsamp);
		// attack rate coefficients
		c->a0 = (fltype)(0.0377*f);
		c->a1 = (fltype)(10.73*f+1);
		c->a2 = (fltype)(-17.57*f);
		c->a3 = (fltype)(7.42*f);

		Bits step_skip = attackrate*4 + c->toff;
		Bits steps = step_skip >> 2;
		c->env_step_a = (1<<(steps<=12?12-steps:0))-1;

		Bits step_num = (step_skip<=48)?(4-(step_skip&3)):0;
		static Bit8u step_skip_mask[5] = {0xff, 0xfe, 0xee, 0xba, 0xaa}; 
		c->env_step_skip_a = step_skip_mask[step_num];
	} else {
		// attack disabled
		c->a0 = 0.0;
		c->a1 = 1.0;
		c->a2 = 0.0;
		c->a3 = 0.0;
		c->env_step_a = 0;
		c->env_step_skip_a = 0;
	}
}

void OPLChipClass::change_decayrate(Bitu regbase, celltype *c) {
	Bits decayrate = adlibreg[ARC_ATTR_DECR+regbase]&15;
	// decaymul should be 1.0 when decayrate==0
	if (decayrate) {
		fltype f = (fltype)(-7.4493*decrelconst[c->toff&3]*recipsamp);
		c->decaymul = (fltype)(pow(FL2,f*pow(FL2,(fltype)(decayrate+(c->toff>>2)))));
		Bits steps = (decayrate*4 + c->toff) >> 2;
		c->env_step_d = (1<<(steps<=12?12-steps:0))-1;
	} else {
		c->decaymul = 1.0;
		c->env_step_d = 0;
	}
}

void OPLChipClass::change_releaserate(Bitu regbase, celltype *c) {
	Bits releaserate = adlibreg[ARC_SUSL_RELR+regbase]&15;
	// releasemul should be 1.0 when releaserate==0
	if (releaserate) {
		fltype f = (fltype)(-7.4493*decrelconst[c->toff&3]*recipsamp);
		c->releasemul = (fltype)(pow(FL2,f*pow(FL2,(fltype)(releaserate+(c->toff>>2)))));
		Bits steps = (releaserate*4 + c->toff) >> 2;
		c->env_step_r = (1<<(steps<=12?12-steps:0))-1;
	} else {
		c->releasemul = 1.0;
		c->env_step_r = 0;
	}
}

void OPLChipClass::change_sustainlevel(Bitu regbase, celltype *c) {
	Bits sustainlevel = adlibreg[ARC_SUSL_RELR+regbase]>>4;
	// sustainlevel should be 0.0 when sustainlevel==15 (max)
	if (sustainlevel<15) {
		c->sustain_level = (fltype)(pow(FL2,(fltype)sustainlevel * (-FL05)));
	} else {
		c->sustain_level = 0.0;
	}
}

void OPLChipClass::change_waveform(Bitu regbase, celltype *c) {
	// waveform selection
	c->cur_wmask = wavemask[wave_sel[regbase]];
	c->cur_wform = &wavtable[waveform[wave_sel[regbase]]];
	// ??? why that:
//	if (!(adlibreg[1]&0x20)) c->cur_wform = &wavtable[WAVPREC];
	// check this: (might need to be adapted to waveform type)
//	c->t = wavestart[wave_sel[regbase]];
}

void OPLChipClass::change_keepsustain(Bitu regbase, celltype *c) {
	c->sus_keep = (adlibreg[ARC_TVS_KSR_MUL+regbase]&0x20)>0;
	if (c->cf_sel==CF_TYPE_SUS) {
		c->cf_sel = CF_TYPE_SUS_NOKEEP;
	} else if (c->cf_sel==CF_TYPE_SUS_NOKEEP) {
		c->cf_sel = CF_TYPE_SUS;
	}
}

// enable/disable vibrato/tremolo LFO effects
void OPLChipClass::change_vibrato(Bitu regbase, celltype *c) {
	c->vibrato = (adlibreg[ARC_TVS_KSR_MUL+regbase]&0x40)!=0;
	c->tremolo = (adlibreg[ARC_TVS_KSR_MUL+regbase]&0x80)!=0;
}

// change amount of self-feedback
void OPLChipClass::change_feedback(Bitu chanbase, celltype *c) {
	Bits feedback = adlibreg[ARC_FEEDBACK+chanbase]&14;
	if (feedback) c->mfb = (fltype)(pow(FL2,(fltype)((feedback>>1)+5))*(WAVPREC/2048.0)*FL05);
	else c->mfb = 0.0;
}

void OPLChipClass::change_cellfreq(Bitu chanbase, Bitu regbase, celltype *c) {
	// frequency
	Bits frn = ((((Bits)adlibreg[ARC_KON_BNUM+chanbase])&3)<<8) + (Bits)adlibreg[ARC_FREQ_NUM+chanbase];
	// block number/octave
	Bits oct = ((((Bits)adlibreg[ARC_KON_BNUM+chanbase])>>2)&7);
	c->freq_high = (frn>>7)&7;

	// keysplit
	Bits note_sel = (adlibreg[8]>>6)&1;
	c->toff = ((frn>>9)&(note_sel^1)) | ((frn>>8)&note_sel);
	c->toff += (oct<<1);

	// envelope scaling (KSR)
	if (!(adlibreg[ARC_TVS_KSR_MUL+regbase]&0x10)) c->toff >>= 2;

	// 20+a0+b0:
	c->tinc = (fltype)(frn<<oct)*nfrqmul[adlibreg[ARC_TVS_KSR_MUL+regbase]&15];
	// 40+a0+b0:
	fltype vol_in = (fltype)((fltype)(adlibreg[ARC_KSL_OUTLEV+regbase]&63) +
				(fltype)kslmul[adlibreg[ARC_KSL_OUTLEV+regbase]>>6]*ksl[oct][frn>>6]);
	c->vol = (fltype)(pow(FL2,(fltype)(vol_in * -0.125 - 14)));

	// cell frequency changed, care about features that depend on it
	change_attackrate(regbase,c);
	change_decayrate(regbase,c);
	change_releaserate(regbase,c);
}

void OPLChipClass::cellon(Bitu regbase, celltype *c) {
	Bits wselbase = regbase;
	if (wselbase>=ARC_SECONDSET) wselbase -= (ARC_SECONDSET-22);	// second set starts at 22

	c->t = wavestart[wave_sel[wselbase]];

	// start with attack mode (as this is a off-on transition of the cellon bit)
	c->cf_sel = CF_TYPE_ATT;
}

void OPLChipClass::adlib_init(Bits samplerate,bool highprec/*=false*/) {
	Bits i, j, oct;

	ext_samplerate = samplerate;
	int_samplerate = samplerate;

	generator_add = (fltype)INTFREQU/(fltype)int_samplerate;


	memset((void *)adlibreg,0,sizeof(adlibreg));
	memset((void *)cell,0,sizeof(celltype)*MAXCELLS);
	memset((void *)wave_sel,0,sizeof(wave_sel));

	for (i=0;i<MAXCELLS;i++) {
		cell[i].cf_sel = CF_TYPE_OFF;
		cell[i].amp = 0.0;
		cell[i].step_amp = 0.0;
		cell[i].vol = 0.0;
		cell[i].t = 0.0;
		cell[i].tinc = 0.0;
		cell[i].toff = 0;
		cell[i].cur_wmask = wavemask[0];
		cell[i].cur_wform = &wavtable[waveform[0]];
		cell[i].freq_high = 0;

		cell[i].generator_pos = 0.0;
		cell[i].cur_env_step = 0;
		cell[i].env_step_a = 0;
		cell[i].env_step_d = 0;
		cell[i].env_step_r = 0;
		cell[i].step_skip_pos = 0;
		cell[i].env_step_skip_a = 0;
	}

	recipsamp = FL1 / (fltype)int_samplerate;
	for (i=15;i>=0;i--) nfrqmul[i] = (fltype)(frqmul[i]*INTFREQU/512.0*recipsamp*(WAVPREC/2048.0));

	status = 0;
	index = 0;
	timer[0] = 0;
	timer[1] = 0;


	// create vibrato table
	vib_table[0] = 8;
	vib_table[1] = 4;
	vib_table[2] = 0;
	vib_table[3] = -4;
	for (i=4; i<VIBTAB_SIZE; i++) vib_table[i] = vib_table[i-4]*-1;

	// vibrato at 6.1 ?? (opl3 docs say 6.1, opl4 docs say 6.0, y8950 docs say 6.4)
	vibtab_add = (fltype)((fltype)VIBTAB_SIZE * VIB_FREQ / (fltype)int_samplerate);
	vibtab_pos = 0.0;

	for (i=0; i<FIFOSIZE; i++) val_const[i] = 1.0;


	// create tremolo table
	Bit32s trem_table_int[TREMTAB_SIZE];
	for (i=0; i<14; i++)	trem_table_int[i] = i-13;		// upwards (13 to 26 -> -0.5/6 to 0)
	for (i=14; i<41; i++)	trem_table_int[i] = -i+14;		// downwards (26 to 0 -> 0 to -1/6)
	for (i=41; i<53; i++)	trem_table_int[i] = i-40-26;	// upwards (1 to 12 -> -1/6 to -0.5/6)

	for (i=0; i<TREMTAB_SIZE; i++) {
		trem_table[i]=(fltype)(((fltype)trem_table_int[i])*4.8/26.0/6.0);				// 4.8db
		trem_table[TREMTAB_SIZE+i]=(fltype)((fltype)((Bit32s)(trem_table_int[i]/4))*1.2/7.5/6.0);	// 1.2db (?)
//		trem_table[i]=(fltype)(trem_table_int[(i)&(~3)])*1.0/13.0/6.0;					// 1.0db

		trem_table[i] = (fltype)(pow(FL2,trem_table[i]));
		trem_table[TREMTAB_SIZE+i] = (fltype)(pow(FL2,trem_table[TREMTAB_SIZE+i]));
	}

	// tremolo at 3.7
	tremtab_add = (fltype)((fltype)TREMTAB_SIZE * TREM_FREQ / (fltype)int_samplerate);
	tremtab_pos = 0.0;


	static Bitu initfirstime = 0;
	if (!initfirstime) {
		initfirstime = 1;

		// create waveform tables
		for (i=0;i<(WAVPREC>>1);i++) {
			wavtable[(i<<1)  +WAVPREC]	= (Bit16s)(16384*sin((fltype)((i<<1)  )*PI*2/WAVPREC));
			wavtable[(i<<1)+1+WAVPREC]	= (Bit16s)(16384*sin((fltype)((i<<1)+1)*PI*2/WAVPREC));
			wavtable[i]					= wavtable[(i<<1)  +WAVPREC];
			// table to be verified, alternative: (zero-less)
/*			wavtable[(i<<1)  +WAVPREC]	= (Bit16s)(16384*sin((fltype)(((i*2+1)<<1)-1)*PI/WAVPREC));
			wavtable[(i<<1)+1+WAVPREC]	= (Bit16s)(16384*sin((fltype)(((i*2+1)<<1)  )*PI/WAVPREC));
			wavtable[i]					= wavtable[(i<<1)-1+WAVPREC]; */
		}
		for (i=0;i<(WAVPREC>>3);i++) {
			wavtable[i+(WAVPREC<<1)]		= wavtable[i+(WAVPREC>>3)]-16384;
			wavtable[i+((WAVPREC*17)>>3)]	= wavtable[i+(WAVPREC>>2)]+16384;
		}

		// key scale level table verified ([table in book]*8/3)
		ksl[7][0] = 0;	ksl[7][1] = 24;	ksl[7][2] = 32;	ksl[7][3] = 37;
		ksl[7][4] = 40;	ksl[7][5] = 43;	ksl[7][6] = 45;	ksl[7][7] = 47;
		ksl[7][8] = 48;
		for (i=9;i<16;i++) ksl[7][i] = (Bit8u)(i+41);
		for (j=6;j>=0;j--) {
			for (i=0;i<16;i++) {
				oct = (Bits)ksl[j+1][i]-8;
				if (oct < 0) oct = 0;
				ksl[j][i] = (Bit8u)oct;
			}
		}
	}

}


void adlib_timeout(Bitu val) {
	if ((val&1)==0) {
		// timer1
		oplchip[(val>>1)&1]->status |= 0xc0;
		float delay = (float)(80.0*(256-oplchip[(val>>1)&1]->timer[0])/1000.0);
// 		PIC_AddEvent(adlib_timeout,delay,val);
	} else {
		// timer2
		oplchip[(val>>1)&1]->status |= 0xa0;
		float delay = (float)(320.0*(256-oplchip[(val>>1)&1]->timer[1])/1000.0);
// 		PIC_AddEvent(adlib_timeout,delay,val);
	}
}


void OPLChipClass::adlib_write(Bitu idx, Bit8u val, Bitu second_set) {
	if (((adlibreg[0x105]&1)==0) && (idx!=5)) second_set = 0;
	idx += second_set;		// add 0x100 for second register set
	Bit8u old_val = adlibreg[idx];
	adlibreg[idx] = val;

	switch (idx&0xf0) {
	case ARC_CONTROL:
		// here we check for the second set registers, too
		switch (idx) {
		case 0x02:	// timer1 counter
		case 0x03:	// timer2 counter
			timer[idx&1] = val;
			break;
		case 0x04:
			// IRQ reset, timer mask/start
			if (val&0x80) {
				// clear IRQ bit in status register
				status &= ~0x7f;
			} else {
				status = 0;
				int chip_sel_val = (this->chip_num?2:0);
				// check timer1
				if ((val&1) > (old_val&1)) {			// transition 0->1, start timer
// 					PIC_RemoveSpecificEvents(adlib_timeout,0|chip_sel_val);
					// 80 mysec timer resolution
					float delay = (float)(80.0*(256-timer[0])/1000.0);
// 					PIC_AddEvent(adlib_timeout,delay,0|chip_sel_val);
				} else if ((val&1) < (old_val&1)) {		// transition 1->0, cancel timer
// 					PIC_RemoveSpecificEvents(adlib_timeout,0|chip_sel_val);
				}
				// check timer2
				if ((val&2) > (old_val&2)) {			// transition 0->1, start timer
// 					PIC_RemoveSpecificEvents(adlib_timeout,1|chip_sel_val);
					// 320 mysec timer resolution
					float delay = (float)(320.0*(256-timer[1])/1000.0);
// 					PIC_AddEvent(adlib_timeout,delay,1|chip_sel_val);
				} else if ((val&2) < (old_val&2)) {		// transition 1->0, cancel timer
// 					PIC_RemoveSpecificEvents(adlib_timeout,1|chip_sel_val);
				}
			}
			break;
		case 0x08:
			// CSW, note select
			break;
		default:
			break;
		}
		break;
	case ARC_TVS_KSR_MUL:
	case ARC_TVS_KSR_MUL+0x10: {
		// tremolo/vibrato/sustain keeping enabled; key scale rate; frequency multiplication
		int num = idx&7;
		Bitu base = (idx-ARC_TVS_KSR_MUL)&0xff;
		if ((num<6) && (base<22)) {
			Bitu modcell = regbase2modcell[second_set?(base+22):base];
			Bitu regbase = base+second_set;
			Bitu chanbase = second_set?(modcell-18+ARC_SECONDSET):modcell;

			// change tremolo/vibrato and sustain keeping of this cell
			celltype* ccellptr = &cell[modcell+((num<3) ? 0 : 9)];
			change_keepsustain(regbase,ccellptr);
			change_vibrato(regbase,ccellptr);

			// change frequency calculations of this cell as
			// key scale rate and frequency multiplicator can be changed
			change_cellfreq(chanbase,base,ccellptr);
		}
		}
		break;
	case ARC_KSL_OUTLEV:
	case ARC_KSL_OUTLEV+0x10: {
		// key scale level; output rate
		int num = idx&7;
		Bitu base = (idx-ARC_KSL_OUTLEV)&0xff;
		if ((num<6) && (base<22)) {
			Bitu modcell = regbase2modcell[second_set?(base+22):base];
			Bitu chanbase = second_set?(modcell-18+ARC_SECONDSET):modcell;

			// change frequency calculations of this cell as
			// key scale level and output rate can be changed
			celltype* ccellptr = &cell[modcell+((num<3) ? 0 : 9)];
			change_cellfreq(chanbase,base,ccellptr);
		}
		}
		break;
	case ARC_ATTR_DECR:
	case ARC_ATTR_DECR+0x10: {
		// attack/decay rates
		int num = idx&7;
		Bitu base = (idx-ARC_ATTR_DECR)&0xff;
		if ((num<6) && (base<22)) {
			Bitu modcell = regbase2modcell[second_set?(base+22):base];
			Bitu regbase = base+second_set;

			// change attack rate and decay rate of this cell
			celltype* ccellptr = &cell[modcell+((num<3) ? 0 : 9)];
			change_attackrate(regbase,ccellptr);
			change_decayrate(regbase,ccellptr);
		}
		}
		break;
	case ARC_SUSL_RELR:
	case ARC_SUSL_RELR+0x10: {
		// sustain level; release rate
		int num = idx&7;
		Bitu base = (idx-ARC_SUSL_RELR)&0xff;
		if ((num<6) && (base<22)) {
			Bitu modcell = regbase2modcell[second_set?(base+22):base];
			Bitu regbase = base+second_set;

			// change sustain level and release rate of this cell
			celltype* ccellptr = &cell[modcell+((num<3) ? 0 : 9)];
			change_releaserate(regbase,ccellptr);
			change_sustainlevel(regbase,ccellptr);
		}
		}
		break;
	case ARC_FREQ_NUM: {
		// 0xa0-0xa8 low8 frequency
		Bitu base = (idx-ARC_FREQ_NUM)&0xff;
		if (base<9) {
			Bits cellbase = second_set?(base+18):base;
			// regbase of modulator:
			Bits modbase = modulatorbase[base]+second_set;

			Bitu chanbase = base+second_set;

			change_cellfreq(chanbase,modbase,&cell[cellbase]);
			change_cellfreq(chanbase,modbase+3,&cell[cellbase+9]);
		}
		}
		break;
	case ARC_KON_BNUM: {
		if (idx == ARC_PERC_MODE) {
			if ((val&16) > (old_val&16)) {		//BassDrum
				// OK
				cellon(16,&cell[6]);
				change_cellfreq(6,16,&cell[6]);
				// OK
				cellon(16+3,&cell[6+9]);
				change_cellfreq(6,16+3,&cell[6+9]);
			}
			if ((val&8) > (old_val&8)) {		//Snare
				cellon(20,&cell[16]);
				change_cellfreq(16,20,&cell[16]);
				cell[16].tinc *= 2*(nfrqmul[adlibreg[ARC_TVS_KSR_MUL+17]&15] / nfrqmul[adlibreg[ARC_TVS_KSR_MUL+20]&15]);
				// ??? :
				if (((adlibreg[ARC_WAVE_SEL+20]&7) >= 3) && ((adlibreg[ARC_WAVE_SEL+20]&7) <= 5)) cell[16].vol = 0;
			}
			if ((val&4) > (old_val&4)) {		//TomTom
				// OK
				cellon(18,&cell[8]);
				change_cellfreq(8,18,&cell[8]);
			}
			if ((val&2) > (old_val&2)) {		//Cymbal
				cellon(21,&cell[17]);
				change_cellfreq(17,21,&cell[17]);

				cell[17].cur_wmask = wavemask[5];
				cell[17].cur_wform = &wavtable[waveform[5]];
				cell[17].tinc *= 16;

				//cell[17].cur_wform = &wavtable[WAVPREC]; cell[17].cur_wmask = 0;
				//if (((adlibreg[21+0xe0]&7) == 0) || ((adlibreg[21+0xe0]&7) == 6))
				//   cell[17].cur_wform = &wavtable[(WAVPREC*7)>>2];
				//if (((adlibreg[21+0xe0]&7) == 2) || ((adlibreg[21+0xe0]&7) == 3))
				//   cell[17].cur_wform = &wavtable[(WAVPREC*5)>>2];
			}
			if ((val&1) > (old_val&1)) {	//Hihat
				cellon(17,&cell[7]);
				change_cellfreq(7,17,&cell[7]);
				Bitu hval = adlibreg[ARC_WAVE_SEL+17]&7;
				if ((hval == 1) || (hval == 4) || (hval == 5) || (hval == 7)) cell[7].vol = 0;
				if (hval == 6) {
					cell[7].cur_wmask = 0;
					cell[7].cur_wform = &wavtable[(WAVPREC*7)>>2];
				}
			}

			break;
		}
		// regular 0xb0-0xb8
		Bitu base = (idx-ARC_KON_BNUM)&0xff;
		if (base<9) {
			Bits cellbase = second_set?(base+18):base;
			// regbase of modulator:
			Bits modbase = modulatorbase[base]+second_set;

			if ((val&32) > (old_val&32)) {
				// key switched ON
				cellon(modbase,&cell[cellbase]);		// modulator (if 2op)
				cellon(modbase+3,&cell[cellbase+9]);	// carrier (if 2op)
			} else if ((val&32) < (old_val&32)) {
				// key switched OFF
				if (cell[cellbase].cf_sel!=CF_TYPE_OFF) cell[cellbase].cf_sel = CF_TYPE_REL;
				if (cell[cellbase+9].cf_sel!=CF_TYPE_OFF) cell[cellbase+9].cf_sel = CF_TYPE_REL;
			}

			Bitu chanbase = base+second_set;

			// change frequency calculations of modulator and carrier (2op) as
			// the frequency of the channel has changed
			change_cellfreq(chanbase,modbase,&cell[cellbase]);
			change_cellfreq(chanbase,modbase+3,&cell[cellbase+9]);
		}
		}
		break;
	case ARC_FEEDBACK: {
		// 0xc0-0xc8 feedback/modulation type (AM/FM)
		Bitu base = (idx-ARC_FEEDBACK)&0xff;
		if (base<9) {
			Bits cellbase = second_set?(base+18):base;
			Bitu chanbase = base+second_set;
			change_feedback(chanbase,&cell[cellbase]);
		}
		}
		break;
	case ARC_WAVE_SEL:
	case ARC_WAVE_SEL+0x10: {
		int num = idx&7;
		Bitu base = (idx-ARC_WAVE_SEL)&0xff;
		if ((num<6) && (base<22)) {
			if (adlibreg[0x01]&0x20) {
				// wave selection enabled, change waveform
				wave_sel[base] = val&3;
				celltype* ccellptr = &cell[regbase2modcell[base]+((num<3) ? 0 : 9)];
				change_waveform(base,ccellptr);
			}
		}
		}
		break;
	default:
		break;
	}
}


Bitu adlib_reg_read(Bitu oplnum, Bitu port) {
	// opl2-detection routines require ret&6 to be 6
	if ((port&1)==0) {
		return oplchip[oplnum&1]->status|6;
	}
	return 0xff;
}

void adlib_reg_write(Bitu oplnum, Bitu port, Bit8u val) {
	static Bitu is_second_set = 0;
	if ((port&1)==0) {
		// index written
		oplchip[oplnum&1]->index = val;
	} else {
		// data written
		oplchip[oplnum&1]->adlib_write(oplchip[oplnum]->index,val,is_second_set);
	}
}


static void clipit16(fltype f, Bit16s* a) {
	if (f>32766.5) *a=32767;
	else if (f<-32767.5) *a=-32768;
	else *a=(Bit16u)f;
}

// be careful with this
// uses cptr and chanval, outputs into outbufl(/outbufr)
// for opl3 check if opl3-mode is enabled (which uses stereo panning)
#undef CHANVAL_OUT
#define CHANVAL_OUT									\
	outbufl[i] += chanval;

void OPLChipClass::adlib_getsample(Bit16s* sndptr, Bits numsamples) {
	Bits i, j, endsamples;
	celltype* cptr;

	fltype outbufl[FIFOSIZE];

	// vibrato/tremolo lookup tables (global to possibly be used by all cells)
	Bit32s vib_lut[FIFOSIZE];
	fltype trem_lut[FIFOSIZE];

	Bits _snarek = 0;

	Bits samples_to_process = numsamples;

	for (Bits cursmp=0;cursmp<samples_to_process;cursmp+=endsamples) {
		endsamples = samples_to_process-cursmp;
		if (endsamples>FIFOSIZE) endsamples = FIFOSIZE;

		memset((void*)&outbufl,0,endsamples*sizeof(fltype));

		// calculate vibrato/tremolo lookup tables
		for (i=0;i<endsamples;i++) {
			// cycle through vibrato table
			vibtab_pos += vibtab_add;
			if (vibtab_pos>=VIBTAB_SIZE) vibtab_pos-=VIBTAB_SIZE;
			vib_lut[i] = vib_table[(Bit32s)vibtab_pos];		// 14cents (14100 of a semitone)

			// cycle through tremolo table
			tremtab_pos += tremtab_add;
			if (tremtab_pos>=TREMTAB_SIZE) tremtab_pos-=TREMTAB_SIZE;
			if (adlibreg[ARC_PERC_MODE]&0x80) trem_lut[i] = trem_table[(Bit32s)tremtab_pos];
			else trem_lut[i] = trem_table[TREMTAB_SIZE+(Bit32s)tremtab_pos];
		}
		if ((adlibreg[ARC_PERC_MODE]&0x40)==0) {
			for (i=0;i<endsamples;i++) vib_lut[i]/=2;		// 7cents only
		}

		if (adlibreg[ARC_PERC_MODE]&0x20) {
			//BassDrum
			cptr = &cell[6];
			if (adlibreg[ARC_FEEDBACK+6]&1) {
				// additive synthesis
				if (cptr[9].cf_sel != CF_TYPE_OFF) {
					if (cptr[9].vibrato) {
						vibval1 = vibval_var1;
						for (i=0;i<endsamples;i++)
							vibval1[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[9].freq_high/8))*0.0014)+FL1;
					} else vibval1 = val_const;
					if (cptr[9].tremolo) tremval1 = trem_lut;	// tremolo enabled, use table
					else tremval1 = val_const;

					// calculate channel output
					for (i=0;i<endsamples;i++) {
						cfuncs[cptr[9].cf_sel](&cptr[9],0.0,vibval1[i],tremval1[i]);
						
						fltype chanval = cptr[9].val*2;
						CHANVAL_OUT
					}
				}
			} else {
				// frequency modulation
				if ((cptr[9].cf_sel != CF_TYPE_OFF) || (cptr[0].cf_sel != CF_TYPE_OFF)) {
					if ((cptr[0].vibrato) && (cptr[0].cf_sel != CF_TYPE_OFF)) {
						vibval1 = vibval_var1;
						for (i=0;i<endsamples;i++)
							vibval1[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[0].freq_high/8))*0.0014)+FL1;
					} else vibval1 = val_const;
					if ((cptr[9].vibrato) && (cptr[9].cf_sel != CF_TYPE_OFF)) {
						vibval2 = vibval_var2;
						for (i=0;i<endsamples;i++)
							vibval2[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[9].freq_high/8))*0.0014)+FL1;
					} else vibval2 = val_const;
					if (cptr[0].tremolo) tremval1 = trem_lut;	// tremolo enabled, use table
					else tremval1 = val_const;
					if (cptr[9].tremolo) tremval1 = trem_lut;	// tremolo enabled, use table
					else tremval1 = val_const;

					// calculate channel output
					for (i=0;i<endsamples;i++) {
						cfuncs[cptr[0].cf_sel](&cptr[0],(cptr[0].lastval+cptr[0].val)*cptr[0].mfb,vibval1[i],tremval1[i]);
						cfuncs[cptr[9].cf_sel](&cptr[9],cptr[0].val*MODFACTOR,vibval2[i],tremval2[i]);
						
						fltype chanval = cptr[9].val*2;
						CHANVAL_OUT
					}
				}
			}

			//Snare/Hihat (j=7), Cymbal/TomTom (j=8)
			if ((cell[7].cf_sel != CF_TYPE_OFF) || (cell[8].cf_sel != CF_TYPE_OFF) ||
				(cell[16].cf_sel != CF_TYPE_OFF) || (cell[17].cf_sel != CF_TYPE_OFF)) {

				cptr = &cell[7];
				if ((cptr[0].vibrato) && (cptr[0].cf_sel != CF_TYPE_OFF)) {
					vibval1 = vibval_var1;
					for (i=0;i<endsamples;i++)
						vibval1[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[0].freq_high/8))*0.0014)+FL1;
				} else vibval1 = val_const;
				if ((cptr[9].vibrato) && (cptr[9].cf_sel == CF_TYPE_OFF)) {
					vibval2 = vibval_var2;
					for (i=0;i<endsamples;i++)
						vibval2[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[9].freq_high/8))*0.0014)+FL1;
				} else vibval2 = val_const;

				if (cptr[0].tremolo) tremval1 = trem_lut;	// tremolo enabled, use table
				else tremval1 = val_const;
				if (cptr[9].tremolo) tremval2 = trem_lut;	// tremolo enabled, use table
				else tremval2 = val_const;

				cptr = &cell[8];
				if ((cptr[0].vibrato) && (cptr[0].cf_sel != CF_TYPE_OFF)) {
					vibval3 = vibval_var1;
					for (i=0;i<endsamples;i++)
						vibval3[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[0].freq_high/8))*0.0014)+FL1;
				} else vibval3 = val_const;
				if ((cptr[9].vibrato) && (cptr[9].cf_sel == CF_TYPE_OFF)) {
					vibval4 = vibval_var2;
					for (i=0;i<endsamples;i++)
						vibval4[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[9].freq_high/8))*0.0014)+FL1;
				} else vibval4 = val_const;

				if (cptr[0].tremolo) tremval3 = trem_lut;	// tremolo enabled, use table
				else tremval3 = val_const;
				if (cptr[9].tremolo) tremval4 = trem_lut;	// tremolo enabled, use table
				else tremval4 = val_const;

				// calculate channel output
				for (i=0;i<endsamples;i++) {
					// rewrite/check this stuff (phase):
					_snarek = _snarek*1664525+1013904223;
					Bits _snarek1 = _snarek&((WAVPREC>>1)-1);
					Bits _snarek2 = _snarek&(WAVPREC-1);
					Bits _snarek3 = _snarek&((WAVPREC>>3)-1);
					float snarek1 = *((float*)&_snarek1);
					float snarek2 = *((float*)&_snarek2);
					float snarek3 = *((float*)&_snarek3);

					cptr = &cell[7];
					cfuncs[cptr[9].cf_sel](&cptr[9],(fltype)snarek1,vibval2[i],tremval2[i]);	//Snare
					cfuncs[cptr[0].cf_sel](&cptr[0],(fltype)snarek2,vibval1[i],tremval1[i]);	//Hihat
					fltype chanval = (cptr[0].val + cptr[9].val)*2;
					CHANVAL_OUT
					
					cptr = &cell[8];
					cfuncs[cptr[9].cf_sel](&cptr[9],(fltype)snarek3,vibval4[i],tremval4[i]);	//Cymbal
					cfuncs[cptr[0].cf_sel](&cptr[0],0.0,vibval3[i],tremval3[i]);				//TomTom
					chanval = (cptr[0].val + cptr[9].val)*2;
					CHANVAL_OUT
				}
			}
		}

		Bitu max_channel = NUM_CHANNELS;
		for (j=max_channel-1;j>=0;j--) {
			// skip percussion cells
			if ((adlibreg[ARC_PERC_MODE]&0x20) && (j >= 6) && (j < 9)) continue;

			Bitu k;
			cptr = &cell[j];
			k = j;

			// check for FM/AM
			if (adlibreg[ARC_FEEDBACK+k]&1) {
				// 2op additive synthesis
				if ((cptr[9].cf_sel == CF_TYPE_OFF) && (cptr[0].cf_sel == CF_TYPE_OFF)) continue;
				if ((cptr[0].vibrato) && (cptr[0].cf_sel != CF_TYPE_OFF)) {
					vibval1 = vibval_var1;
					for (i=0;i<endsamples;i++)
						vibval1[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[0].freq_high/8))*0.0014)+FL1;
				} else vibval1 = val_const;
				if ((cptr[9].vibrato) && (cptr[9].cf_sel == CF_TYPE_OFF)) {
					vibval2 = vibval_var2;
					for (i=0;i<endsamples;i++)
						vibval2[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[9].freq_high/8))*0.0014)+FL1;
				} else vibval2 = val_const;
				if (cptr[0].tremolo) tremval1 = trem_lut;	// tremolo enabled, use table
				else tremval1 = val_const;
				if (cptr[9].tremolo) tremval2 = trem_lut;	// tremolo enabled, use table
				else tremval2 = val_const;

				// calculate channel output
				for (i=0;i<endsamples;i++) {
					// carrier1
					cfuncs[cptr[0].cf_sel](&cptr[0],(cptr[0].lastval+cptr[0].val)*cptr[0].mfb,vibval1[i],tremval1[i]);
					// carrier2
					cfuncs[cptr[9].cf_sel](&cptr[9],0.0,vibval2[i],tremval2[i]);

					fltype chanval = cptr[9].val + cptr[0].val;
					CHANVAL_OUT
				}
			} else {
				// 2op frequency modulation
				if ((cptr[9].cf_sel == CF_TYPE_OFF) && (cptr[0].cf_sel == CF_TYPE_OFF)) continue;
				if ((cptr[0].vibrato) && (cptr[0].cf_sel != CF_TYPE_OFF)) {
					vibval1 = vibval_var1;
					for (i=0;i<endsamples;i++)
						vibval1[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[0].freq_high/8))*0.0014)+FL1;
				} else vibval1 = val_const;
				if ((cptr[9].vibrato) && (cptr[9].cf_sel != CF_TYPE_OFF)) {
					vibval2 = vibval_var2;
					for (i=0;i<endsamples;i++)
						vibval2[i] = (fltype)(((Bit32s)(vib_lut[i]*cptr[9].freq_high/8))*0.0014)+FL1;
				} else vibval2 = val_const;
				if (cptr[0].tremolo) tremval1 = trem_lut;	// tremolo enabled, use table
				else tremval1 = val_const;
				if (cptr[9].tremolo) tremval2 = trem_lut;	// tremolo enabled, use table
				else tremval2 = val_const;

				// calculate channel output
				for (i=0;i<endsamples;i++) {
					// modulator
					cfuncs[cptr[0].cf_sel](&cptr[0],(cptr[0].lastval+cptr[0].val)*cptr[0].mfb,vibval1[i],tremval1[i]);
					// carrier
					cfuncs[cptr[9].cf_sel](&cptr[9],cptr[0].val*MODFACTOR,vibval2[i],tremval2[i]);

					fltype chanval = cptr[9].val;
					CHANVAL_OUT
				}
			}
		}

		// convert to 16bit samples
		for (i=0;i<endsamples;i++)
			clipit16(outbufl[i]*AMPVOL,sndptr++);

	}
}

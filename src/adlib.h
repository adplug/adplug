/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2005 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * adlib.h - AdLib Sound Driver by Stas'M <binarymaster@mail.ru>
 *
 * Based on ADLIB.H by Marc Savary and Dale Glowinski, Ad Lib Inc.
 */

#ifndef H_ADPLUG_ADLIBDRV
#define H_ADPLUG_ADLIBDRV

#include "stdint.h"
#include "player.h"

/* Parameters of each voice: */
#define nbLocParam		14

#define prmKsl			0
#define prmMulti		1
#define prmFeedBack		2		/* use for opr. 0 only */
#define prmAttack		3
#define prmSustain		4
#define prmStaining		5		/* Sustaining ... */
#define prmDecay		6
#define prmRelease		7
#define prmLevel		8
#define prmAm			9
#define prmVib			10
#define prmKsr			11
#define prmFm			12			/* use for opr. 0 only */
#define prmWaveSel		13			/* wave select */

 /* globals parameters: */
#define prmAmDepth		14
#define prmVibDepth		15
#define prmNoteSel		16
#define prmPercussion	17

 /* melodic voice numbers: */
#define vMelo0			0
#define vMelo1			1
#define vMelo2			2
#define vMelo3			3
#define vMelo4			4
#define vMelo5			5
#define vMelo6			6
#define vMelo7			7
#define vMelo8			8

 /* percussive voice numbers: */
#define BD				6
#define SD				7
#define TOM				8
#define CYMB			9
#define HIHAT			10


#define MAX_VOLUME		0x7f
#define MAX_PITCH		0x3fff
#define MID_PITCH		0x2000

#define MID_C			60			/* MIDI standard mid C */
#define CHIP_MID_C		48			/* sound chip mid C */
#define NR_NOTES		96			/* # of notes we can play on chip */

#define TOM_PITCH	24				/* best frequency, in range of 0 to 95 */
#define TOM_TO_SD	7				/* 7 half-tones between voice 7 & 8 */
#define SD_PITCH	(TOM_PITCH + TOM_TO_SD)

#define NR_STEP_PITCH	25			/* 25 steps within a half-tone for pitch bend */
#define ADLIB_INST_LEN	28			/* modulator, carrier, wave select */

#define GetLocPrm(slot, prm) ( (uint8_t)paramSlot[slot][prm] )

class CadlibDriver
{
public:
	CadlibDriver(Copl *opl);
	~CadlibDriver()
	{};

	void SoundWarmInit();
	void SetMode(int mode);
	void SetWaveSel(int state);
	void SetPitchRange(uint8_t pR);
	void SetGParam(int amD, int vibD, int nSel);
	void SetVoiceTimbre(uint8_t voice, uint8_t paramArray[ADLIB_INST_LEN]);
	void SetVoiceVolume(uint8_t voice, uint8_t volume);
	void SetVoicePitch(uint8_t voice, uint16_t pitchBend);
	void NoteOn(uint8_t voice, int pitch);
	void NoteOff(uint8_t voice);

private:
	Copl *opl;

	void InitSlotVolume();
	void InitFNums();
	void SoundChut(int voice);
	void SetFreq(uint8_t voice, int pitch, uint8_t keyOn);
	void SndSAmVibRhythm();
	void SndSNoteSel();
	void SndSKslLevel(uint8_t slot);
	void SndSFeedFm(uint8_t slot);
	void SndSAttDecay(uint8_t slot);
	void SndSSusRelease(uint8_t slot);
	void SndSAVEK(uint8_t slot);
	void SndWaveSelect(uint8_t slot);
	void SndSetAllPrm(uint8_t slot);
	void SetSlotParam(uint8_t slot, uint8_t * param, uint8_t waveSel);
	void SetCharSlotParam(uint8_t slot, char * cParam, uint8_t waveSel);
	void InitSlotParams();
	void SetFNum(uint16_t * fNumVec, int num, int den);
	void ChangePitch(int voice, int pitchBend);
	long CalcPremFNum(int numDeltaDemiTon, int denDeltaDemiTon);

	uint16_t fNumNotes[NR_STEP_PITCH][12];
	int		halfToneOffset[11];
	uint16_t * fNumFreqPtr[11];
	int	 	pitchRange;			/* pitch variation, half-tone [+1,+12] */
	int 	pitchRangeStep;		/* == pitchRange * NR_STEP_PITCH */
	int		modeWaveSel;		/* != 0 if used with the 'wave-select' parameters */

	char percBits;				/* control bits of percussive voices */
	char notePitch[11];			/* pitch of last note-on of each voice */
	char voiceKeyOn[11];		/* state of keyOn bit of each voice */

	char noteDIV12[96];			/* table of (0..95) DIV 12 */
	char noteMOD12[96];			/* table of (0..95) MOD 12 */

	char slotRelVolume[18];		/* relative volume of slots */

	typedef char SLOT_PARAM;
	SLOT_PARAM paramSlot[18][nbLocParam];	/* all the parameters of slots...  */

	char amDepth;			/* chip global parameters .. */
	char vibDepth;			/* ... */
	char noteSel;			/* ... */
	char percussion;		/* percussion mode parameter */

protected:
	const char percMasks[5] = {0x10, 0x08, 0x04, 0x02, 0x01};

	/* definition of the ELECTRIC-PIANO voice (opr0 & opr1) */
	char pianoParamsOp0[nbLocParam] =
		{1, 1, 3, 15, 5, 0, 1, 3, 15, 0, 0, 0, 1, 0 };
	char pianoParamsOp1[nbLocParam] =
		{0, 1, 1, 15, 7, 0, 2, 4, 0, 0, 0, 1, 0, 0 };

	/* definition of default percussive voices: */
	char bdOpr0[nbLocParam] =
		{ 0,  0, 0, 10,  4, 0, 8, 12, 11, 0, 0, 0, 1, 0 };
	char bdOpr1[nbLocParam] =
		{ 0,  0, 0, 13,  4, 0, 6, 15,  0, 0, 0, 0, 1, 0 };
	char sdOpr[nbLocParam] =
		{ 0, 12, 0, 15, 11, 0, 8,  5,  0, 0, 0, 0, 0, 0 };
	char tomOpr[nbLocParam] =
		{ 0,  4, 0, 15, 11, 0, 7,  5,  0, 0, 0, 0, 0, 0 };
	char cymbOpr[nbLocParam] =
		{ 0,  1, 0, 15, 11, 0, 5,  5,  0, 0, 0, 0, 0, 0 };
	char hhOpr[nbLocParam] =
		{ 0,  1, 0, 15, 11, 0, 7,  5,  0, 0, 0, 0, 0, 0 };

	/* Slot numbers as a function of the voice and the operator.
		( melodic only)
	*/
	char slotVoice[9][2] = {
		{ 0, 3 },	/* voix 0 */
		{ 1, 4 },	/* 1 */
		{ 2, 5 },	/* 2 */
		{ 6, 9 },	/* 3 */
		{ 7, 10 },	/* 4 */
		{ 8, 11 },	/* 5 */
		{ 12, 15 },	/* 6 */
		{ 13, 16 },	/* 7 */
		{ 14, 17 }	/* 8 */
	};

	/* Slot numbers for the percussive voices.
		0 indicates that there is only one slot.
	*/
	char slotPerc[5][2] = {
		{ 12, 15 },		/* Bass Drum: slot 12 et 15 */
		{ 16, 0 },		/* SD: slot 16 */
		{ 14, 0 },		/* TOM: slot 14 */
		{ 17, 0 },		/* TOP-CYM: slot 17 */
		{ 13, 0 }		/* HH: slot 13 */
	};

	/*
		This table gives the offset of each slot within the chip.
		offset = fn( slot)
	*/
	char offsetSlot[18] = {
		0,  1,  2,  3,  4,  5,
		8,  9, 10, 11, 12, 13,
		16, 17, 18, 19, 20, 21
	};

	/* This table indicates if the slot is a modulator (0) or a carrier (1).
		opr = fn( slot)
	*/
	char operSlot[18] = {
		0, 0, 0,		/* 1 2 3 */
		1, 1, 1,		/* 4 5 6 */
		0, 0, 0, 		/* 7 8 9 */
		1, 1, 1, 		/* 10 11 12 */
		0, 0, 0, 		/* 13 14 15 */
		1, 1, 1,		/* 16 17 18 */
	};

	/* This table gives the voice number associated with each slot.
		(melodic mode only)
		voice = fn( slot)
	*/
	char voiceSlot[18] = {
		0, 1, 2,
		0, 1, 2,
		3, 4, 5,
		3, 4, 5,
		6, 7, 8,
		6, 7, 8,
	};
};

#endif

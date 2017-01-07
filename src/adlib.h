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
 * adlib.h - AdLib Sound Driver by Stas'M <binarymaster@mail.ru> and Jepael
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

#define MAX_VOICES		11
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
#define ADLIB_OPER_LEN	13			/* operator length */
#define ADLIB_INST_LEN	(ADLIB_OPER_LEN * 2 + 2)	/* modulator, carrier, mod/car wave select */

#define GetLocPrm(slot, prm) ( (uint8_t)paramSlot[slot][prm] )

class CadlibDriver
{
public:
	CadlibDriver(Copl *newopl)
		: opl(newopl)
	{};
	~CadlibDriver()
	{};

	void SoundWarmInit();
	void SetMode(int mode);
	void SetWaveSel(int state);
	void SetPitchRange(uint8_t pR);
	void SetGParam(int amD, int vibD, int nSel);
	void SetVoiceTimbre(uint8_t voice, int16_t * paramArray);
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
	void SetSlotParam(uint8_t slot, int16_t * param, uint8_t waveSel);
	void SetCharSlotParam(uint8_t slot, char * cParam, uint8_t waveSel);
	void InitSlotParams();
	void SetFNum(uint16_t * fNumVec, int num, int den);
	void ChangePitch(int voice, int pitchBend);
	long CalcPremFNum(int numDeltaDemiTon, int denDeltaDemiTon);

	uint16_t fNumNotes[NR_STEP_PITCH][12];
	int		halfToneOffset[MAX_VOICES];
	uint16_t * fNumFreqPtr[MAX_VOICES];
	int	 	pitchRange;			/* pitch variation, half-tone [+1,+12] */
	int 	pitchRangeStep;		/* == pitchRange * NR_STEP_PITCH */
	int		modeWaveSel;		/* != 0 if used with the 'wave-select' parameters */

	char percBits;				/* control bits of percussive voices */
	char notePitch[MAX_VOICES];	/* pitch of last note-on of each voice */
	char voiceKeyOn[MAX_VOICES];	/* state of keyOn bit of each voice */

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
	static const char percMasks[5];
	static char pianoParamsOp0[nbLocParam];
	static char pianoParamsOp1[nbLocParam];
	static char bdOpr0[nbLocParam];
	static char bdOpr1[nbLocParam];
	static char sdOpr[nbLocParam];
	static char tomOpr[nbLocParam];
	static char cymbOpr[nbLocParam];
	static char hhOpr[nbLocParam];
	static char slotVoice[9][2];
	static char slotPerc[5][2];
	static char offsetSlot[18];
	static char operSlot[18];
	static char voiceSlot[18];
};

#endif

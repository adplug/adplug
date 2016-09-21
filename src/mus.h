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
 * mus.h - AdLib MIDI Music File Player by Stas'M <binarymaster@mail.ru>
 *
 * Based on PLAY.C by Marc Savary, Ad Lib Inc.
 *
 * REFERENCES:
 * http://www.shikadi.net/moddingwiki/AdLib_MIDI_Format
 * http://www.shikadi.net/moddingwiki/IMS_Format
 * http://www.shikadi.net/moddingwiki/AdLib_Timbre_Bank_Format
 * http://www.shikadi.net/moddingwiki/AdLib_Instrument_Bank_Format
 * http://www.vgmpf.com/Wiki/index.php?title=MUS_(AdLib)
 * http://www.vgmpf.com/Wiki/index.php?title=SND_(AdLib)
 */

#ifndef H_ADPLUG_MUSPLAYER
#define H_ADPLUG_MUSPLAYER

#include "player.h"
#include "adlib.h"

#define SYSTEM_XOR_BYTE		0xF0
#define EOX_BYTE			0xF7
#define OVERFLOW_BYTE		0xF8
#define STOP_BYTE			0xFC

#define NOTE_OFF_BYTE		0x80
#define NOTE_ON_BYTE		0x90
#define AFTER_TOUCH_BYTE	0xA0
#define CONTROL_CHANGE_BYTE	0xB0
#define PROG_CHANGE_BYTE	0xC0
#define CHANNEL_PRESSURE_BYTE	0xD0
#define PITCH_BEND_BYTE		0xE0

#define ADLIB_CTRL_BYTE	0x7F	/* for System exclusive */
#define TEMPO_CTRL_BYTE	0

#define NR_VOICES			11
#define TUNE_NAME_SIZE		30
#define FILLER_SIZE			8
#define TIMBRE_NAME_SIZE	9
#define TIMBRE_DEF_LEN		ADLIB_INST_LEN
#define TIMBRE_DEF_SIZE 	(TIMBRE_DEF_LEN * sizeof(int16_t))
#define OVERFLOW_TICKS		240
#define DEFAULT_TICK_BEAT	240
#define HEADER_LEN			70
#define SND_HEADER_LEN		6
#define IMS_SIGNATURE		0x7777

#define BNK_HEADER_SIZE		28
#define BNK_SIGNATURE_LEN	6
#define BNK_NAME_SIZE		12
#define BNK_INST_SIZE		30

class CmusPlayer: public CPlayer
{
public:
	static CPlayer *factory(Copl *newopl);

	CmusPlayer(Copl *newopl)
		: CPlayer(newopl), data(0)
		{ }
	~CmusPlayer()
	{
		if (data) delete [] data;
		if (insts) delete[] insts;
		if (drv) drv->~CadlibDriver();
	};

	bool load(const std::string &filename, const CFileProvider &fp);
	bool update();
	void rewind(int subsong);

	float getrefresh()
	{
		return timer;
	};

	std::string gettitle()
	{
		return std::string(mH.tuneName);
	};

	std::string gettype()
	{
		char	tmpstr[30];

		if (isIMS)
			sprintf(tmpstr, "AdLib MIDI/IMS Format v%d.%d", mH.majorVersion, mH.minorVersion);
		else
			sprintf(tmpstr, "AdLib MIDI Format v%d.%d", mH.majorVersion, mH.minorVersion);
		return std::string(tmpstr);
	}

	unsigned int getinstruments()
	{
		return insts ? tH.nrTimbre : 0;
	};

	std::string getinstrument(unsigned int n)
	{
		
		return insts && n < tH.nrTimbre ? (insts[n].loaded ? std::string(insts[n].name) : std::string("[N/A] ").append(insts[n].name)) : std::string();
	};

private:
	bool InstsLoaded();
	bool LoadTimbreBank(const std::string fname, const CFileProvider &fp);
	bool FetchTimbreData(const std::string fname, const CFileProvider &fp);
	void SetTempo(uint16_t tempo, uint8_t tickBeat);
	uint32_t GetTicks();
	CadlibDriver *drv;

protected:
	unsigned long	pos;
	bool		songend;
	float		timer, rate;

	/* structure of music file: */
	struct MusHeader {
		uint8_t		majorVersion;
		uint8_t		minorVersion;
		int32_t		tuneId;
		char		tuneName[TUNE_NAME_SIZE];
		uint8_t		tickBeat;
		uint8_t		beatMeasure;
		uint32_t	totalTick;
		uint32_t	dataSize;
		uint32_t	nrCommand;
		uint8_t		filler[FILLER_SIZE];

		uint8_t		soundMode;			/* 0: melodic, 1: percussive */
		uint8_t		pitchBRange;		/* 1 - 12 */
		uint16_t	basicTempo;
		uint8_t		filler2[FILLER_SIZE];

		/* char		data[]; */
	};

	/* structure of timbre bank file: */
	struct TimHeader {
		uint8_t		majorVersion;
		uint8_t		minorVersion;
		uint16_t	nrTimbre;	/* # of definitions in bank. */
		uint16_t	offsetDef;	/* offset in file of first definition */

								/*	char		timbreName[ ][ TIMBRE_NAME_SIZE];  */
								/*	uint16_t	timbreDef[ ][ TIMBRE_DEF_LEN];  */
	};

	/* structure of timbre in memory: */
	struct TimbreRec {
		char	name[TIMBRE_NAME_SIZE];
		bool	loaded;
		uint8_t	data[TIMBRE_DEF_LEN];
	};

	struct		MusHeader mH;			/* header of .MUS file */
	uint8_t *	data;					/* MIDI data */
	struct		TimHeader tH;			/* header of .SND / .TIM file */
	TimbreRec *	insts;					/* instrument definitions */
	bool		isIMS;					/* play as IMS format */

	bool		firstDelay;				/* flag to process first delay */
	uint8_t		status;                 /* running status byte */
	uint8_t		volume[NR_VOICES];		/* actual volume of all voices */
};

#endif
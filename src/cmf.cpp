/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2009 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * cmf.cpp - CMF player by Adam Nielsen <malvineous@shikadi.net>
 *   Subset of CMF reader in MOPL code (Malvineous' OPL player), no seeking etc.
 */

#include <stdint.h> // for uintxx_t
#include <cassert>
#include <math.h> // for pow() etc.
#include <string.h> // for memset
#include "debug.h"
#include "cmf.h"

// OPL register offsets
#define BASE_CHAR_MULT  0x20
#define BASE_SCAL_LEVL  0x40
#define BASE_ATCK_DCAY  0x60
#define BASE_SUST_RLSE  0x80
#define BASE_FNUM_L     0xA0
#define BASE_KEYON_FREQ 0xB0
#define BASE_RHYTHM     0xBD
#define BASE_WAVE       0xE0
#define BASE_FEED_CONN  0xC0

#define OPLBIT_KEYON    0x20 // Bit in BASE_KEYON_FREQ register for turning a note on

// Supplied with a channel, return the offset from a base OPL register for the
// Modulator cell (e.g. channel 4's modulator is at offset 0x09.  Since 0x60 is
// the attack/decay function, register 0x69 will thus set the attack/decay for
// channel 4's modulator.)  (channels go from 0 to 8 inclusive)
#define OPLOFFSET(channel)   (((channel) / 3) * 8 + ((channel) % 3))

// Default instrument bank, ported verbatim (and in the same order) from the
// SBFMDRV reference (viiri/fmdrv "default_inst_bank").  In the original Creative
// driver this bank is only active before a CMF is loaded; once a song is playing
// the file's own instrument table is used instead and program numbers are taken
// modulo the file's instrument count (see load() and the 0xC0 handler below).
// We therefore only use this bank as a fallback for the (unusual) case of a file
// that declares zero instruments.
//
// Each row is the 11-byte instrument record in CMF on-disk order:
//   [0]/[1]  modulator/carrier characteristic + multiplier (reg 0x20)
//   [2]/[3]  modulator/carrier scaling + output level      (reg 0x40)
//   [4]/[5]  modulator/carrier attack + decay              (reg 0x60)
//   [6]/[7]  modulator/carrier sustain + release           (reg 0x80)
//   [8]/[9]  modulator/carrier wave select                 (reg 0xE0)
//   [10]     feedback + connection                         (reg 0xC0)
// Note byte [3] (carrier scaling/output level) is non-zero for several patches
// here, where the previous AdPlug bank forced it to zero.
static const uint8_t cDefaultPatches[] =
"\x21\x21\xD1\x07\xA3\xA4\x46\x25\x00\x00\x0A"
"\x22\x22\x0F\x0F\xF6\xF6\x95\x36\x00\x00\x0A"
"\xE1\xE1\x00\x00\x44\x54\x24\x34\x02\x02\x07"
"\xA5\xB1\xD2\x80\x81\xF1\x03\x05\x00\x00\x02"
"\x71\x22\xC5\x05\x6E\x8B\x17\x0E\x00\x00\x02"
"\x32\x21\x16\x80\x73\x75\x24\x57\x00\x00\x0E"
"\x01\x11\x4F\x00\xF1\xD2\x53\x74\x00\x00\x06"
"\x07\x12\x4F\x00\xF2\xF2\x60\x72\x00\x00\x08"
"\x31\xA1\x1C\x80\x51\x54\x03\x67\x00\x00\x0E"
"\x31\xA1\x1C\x80\x41\x92\x0B\x3B\x00\x00\x0E"
"\x31\x16\x87\x80\xA1\x7D\x11\x43\x00\x00\x08"
"\x30\xB1\xC8\x80\xD5\x61\x19\x1B\x00\x00\x0C"
"\xF1\x21\x01\x0D\x97\xF1\x17\x18\x00\x00\x08"
"\x32\x16\x87\x80\xA1\x7D\x10\x33\x00\x00\x08"
"\x01\x12\x4F\x00\x71\x52\x53\x7C\x00\x00\x0A"
"\x02\x03\x8D\x03\xD7\xF5\x37\x18\x00\x00\x04";

// The Creative driver (SBFMDRV) programs every channel with this default
// instrument at reset, so that untouched channels start from a known register
// state.  Byte layout matches the on-disk CMF instrument record:
//   [0]/[1]  modulator/carrier characteristic + multiplier (reg 0x20)
//   [2]/[3]  modulator/carrier scaling + output level      (reg 0x40)
//   [4]/[5]  modulator/carrier attack + decay              (reg 0x60)
//   [6]/[7]  modulator/carrier sustain + release           (reg 0x80)
//   [8]/[9]  modulator/carrier wave select                 (reg 0xE0)
//   [10]     feedback + connection                         (reg 0xC0)
static const uint8_t cInitInstrument[11] =
	{ 0x01, 0x11, 0x4F, 0x00, 0xF1, 0xF2, 0x53, 0x74, 0x00, 0x00, 0x08 };

// Note -> block/F-number lookup tables, ported verbatim from the SBFMDRV
// reference (viiri/fmdrv).  block_note_tbl maps a MIDI note (0-127) to a byte
// holding the OPL block (octave) in the high nibble and the semitone within the
// octave in the low nibble.  fnum_tbl maps a note index measured in 1/64ths of a
// semitone (768 == 12 semitones * 64) to the 10-bit OPL F-number.  See getFreq().
static const uint8_t block_note_tbl[128] = { 
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
	0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b, 0x7b
};

static const uint16_t fnum_tbl[768] = {
	343, 343, 344, 344, 344, 344, 345, 345, 345, 346, 346, 346,
	347, 347, 347, 348, 348, 348, 349, 349, 349, 349, 350, 350,
	350, 351, 351, 351, 352, 352, 352, 353, 353, 353, 354, 354,
	354, 355, 355, 355, 356, 356, 356, 356, 357, 357, 357, 358,
	358, 358, 359, 359, 359, 360, 360, 360, 361, 361, 361, 362,
	362, 362, 363, 363, 363, 364, 364, 364, 365, 365, 365, 366,
	366, 366, 367, 367, 367, 368, 368, 368, 369, 369, 369, 370,
	370, 370, 371, 371, 371, 372, 372, 372, 373, 373, 373, 374,
	374, 374, 375, 375, 375, 376, 376, 376, 377, 377, 377, 378,
	378, 378, 379, 379, 379, 380, 380, 380, 381, 381, 381, 382,
	382, 382, 383, 383, 384, 384, 384, 385, 385, 385, 386, 386,
	386, 387, 387, 387, 388, 388, 388, 389, 389, 389, 390, 390,
	391, 391, 391, 392, 392, 392, 393, 393, 393, 394, 394, 394,
	395, 395, 395, 396, 396, 397, 397, 397, 398, 398, 398, 399,
	399, 399, 400, 400, 401, 401, 401, 402, 402, 402, 403, 403,
	403, 404, 404, 405, 405, 405, 406, 406, 406, 407, 407, 407,
	408, 408, 409, 409, 409, 410, 410, 410, 411, 411, 412, 412,
	412, 413, 413, 413, 414, 414, 414, 415, 415, 416, 416, 416,
	417, 417, 417, 418, 418, 419, 419, 419, 420, 420, 421, 421,
	421, 422, 422, 422, 423, 423, 424, 424, 424, 425, 425, 425,
	426, 426, 427, 427, 427, 428, 428, 429, 429, 429, 430, 430,
	430, 431, 431, 432, 432, 432, 433, 433, 434, 434, 434, 435,
	435, 436, 436, 436, 437, 437, 438, 438, 438, 439, 439, 440,
	440, 440, 441, 441, 442, 442, 442, 443, 443, 444, 444, 444,
	445, 445, 446, 446, 446, 447, 447, 448, 448, 448, 449, 449,
	450, 450, 450, 451, 451, 452, 452, 452, 453, 453, 454, 454,
	454, 455, 455, 456, 456, 457, 457, 457, 458, 458, 459, 459,
	459, 460, 460, 461, 461, 461, 462, 462, 463, 463, 464, 464,
	464, 465, 465, 466, 466, 467, 467, 467, 468, 468, 469, 469,
	469, 470, 470, 471, 471, 472, 472, 472, 473, 473, 474, 474,
	475, 475, 475, 476, 476, 477, 477, 478, 478, 478, 479, 479,
	480, 480, 481, 481, 481, 482, 482, 483, 483, 484, 484, 485,
	485, 485, 486, 486, 487, 487, 488, 488, 488, 489, 489, 490,
	490, 491, 491, 492, 492, 492, 493, 493, 494, 494, 495, 495,
	496, 496, 496, 497, 497, 498, 498, 499, 499, 500, 500, 501,
	501, 501, 502, 502, 503, 503, 504, 504, 505, 505, 506, 506,
	506, 507, 507, 508, 508, 509, 509, 510, 510, 511, 511, 511,
	512, 512, 513, 513, 514, 514, 515, 515, 516, 516, 517, 517,
	518, 518, 518, 519, 519, 520, 520, 521, 521, 522, 522, 523,
	523, 524, 524, 525, 525, 526, 526, 526, 527, 527, 528, 528,
	529, 529, 530, 530, 531, 531, 532, 532, 533, 533, 534, 534,
	535, 535, 536, 536, 537, 537, 538, 538, 538, 539, 539, 540,
	540, 541, 541, 542, 542, 543, 543, 544, 544, 545, 545, 546,
	546, 547, 547, 548, 548, 549, 549, 550, 550, 551, 551, 552,
	552, 553, 553, 554, 554, 555, 555, 556, 556, 557, 557, 558,
	558, 559, 559, 560, 560, 561, 561, 562, 562, 563, 563, 564,
	564, 565, 565, 566, 566, 567, 567, 568, 568, 569, 569, 570,
	571, 571, 572, 572, 573, 573, 574, 574, 575, 575, 576, 576,
	577, 577, 578, 578, 579, 579, 580, 580, 581, 581, 582, 582,
	583, 584, 584, 585, 585, 586, 586, 587, 587, 588, 588, 589,
	589, 590, 590, 591, 591, 592, 593, 593, 594, 594, 595, 595,
	596, 596, 597, 597, 598, 598, 599, 600, 600, 601, 601, 602,
	602, 603, 603, 604, 604, 605, 606, 606, 607, 607, 608, 608,
	609, 609, 610, 610, 611, 612, 612, 613, 613, 614, 614, 615,
	615, 616, 617, 617, 618, 618, 619, 619, 620, 620, 621, 622,
	622, 623, 623, 624, 624, 625, 626, 626, 627, 627, 628, 628,
	629, 629, 630, 631, 631, 632, 632, 633, 633, 634, 635, 635,
	636, 636, 637, 637, 638, 639, 639, 640, 640, 641, 642, 642,
	643, 643, 644, 644, 645, 646, 646, 647, 647, 648, 649, 649,
	650, 650, 651, 651, 652, 653, 653, 654, 654, 655, 656, 656,
	657, 657, 658, 659, 659, 660, 660, 661, 662, 662, 663, 663,
	664, 665, 665, 666, 666, 667, 668, 668, 669, 669, 670, 671,
	671, 672, 672, 673, 674, 674, 675, 675, 676, 677, 677, 678,
	678, 679, 680, 680, 681, 682, 682, 683, 683, 684, 685, 685
};


CPlayer *CcmfPlayer::factory(Copl *newopl)
{
  return new CcmfPlayer(newopl);
}

CcmfPlayer::CcmfPlayer(Copl *newopl) :
	CPlayer(newopl),
	data(NULL),
	pInstruments(NULL),
	iInstCount(16),
	bPercussive(false),
	iPrevCommand(0)
{
	assert(OPLOFFSET(1-1) == 0x00);
	assert(OPLOFFSET(5-1) == 0x09);
	assert(OPLOFFSET(9-1) == 0x12);
}

CcmfPlayer::~CcmfPlayer()
{
	if (this->data) delete[] data;
	if (this->pInstruments) delete[] pInstruments;
}

bool CcmfPlayer::load(const std::string &filename, const CFileProvider &fp)
{
  binistream *f = fp.open(filename); if(!f) return false;

	char cSig[4];
	f->readString(cSig, 4);
	if (
		(cSig[0] != 'C') ||
		(cSig[1] != 'T') ||
		(cSig[2] != 'M') ||
		(cSig[3] != 'F')
	) {
		// Not a CMF file
		fp.close(f);
		return false;
	}
	uint16_t iVer = f->readInt(2);
	if ((iVer != 0x0101) && (iVer != 0x0100)) {
		AdPlug_LogWrite("CMF file is not v1.0 or v1.1 (reports %d.%d)\n", iVer >> 8 , iVer & 0xFF);
		fp.close(f);
		return false;
	}

	this->cmfHeader.iInstrumentBlockOffset = f->readInt(2);
	this->cmfHeader.iMusicOffset = f->readInt(2);
	this->cmfHeader.iTicksPerQuarterNote = f->readInt(2);
	this->cmfHeader.iTicksPerSecond = f->readInt(2);
	this->cmfHeader.iTagOffsetTitle = f->readInt(2);
	this->cmfHeader.iTagOffsetComposer = f->readInt(2);
	this->cmfHeader.iTagOffsetRemarks = f->readInt(2);

	// This checks will fix crash for a lot of broken files
	// Title, Composer and Remarks blocks usually located before Instrument block
	// But if not this will indicate invalid offset value (sometimes even bigger than filesize)
	if (this->cmfHeader.iTagOffsetTitle >= this->cmfHeader.iInstrumentBlockOffset)
		this->cmfHeader.iTagOffsetTitle = 0;
	if (this->cmfHeader.iTagOffsetComposer >= this->cmfHeader.iInstrumentBlockOffset)
		this->cmfHeader.iTagOffsetComposer = 0;
	if (this->cmfHeader.iTagOffsetRemarks >= this->cmfHeader.iInstrumentBlockOffset)
		this->cmfHeader.iTagOffsetRemarks = 0;

	f->readString((char *)this->cmfHeader.iChannelsInUse, 16);
	if (iVer == 0x0100) {
		this->cmfHeader.iNumInstruments = f->readInt(1);
		this->cmfHeader.iTempo = 0;
	} else { // 0x0101
		this->cmfHeader.iNumInstruments = f->readInt(2);
		this->cmfHeader.iTempo = f->readInt(2);
	}

	// Load the instruments

	f->seek(this->cmfHeader.iInstrumentBlockOffset);
	this->pInstruments = new SBI[
		(this->cmfHeader.iNumInstruments < 128) ? 128 : this->cmfHeader.iNumInstruments
	];  // Always at least 128 available for use

	for (int i = 0; i < this->cmfHeader.iNumInstruments; i++) {
		this->pInstruments[i].op[0].iCharMult = f->readInt(1);
		this->pInstruments[i].op[1].iCharMult = f->readInt(1);
		this->pInstruments[i].op[0].iScalingOutput = f->readInt(1);
		this->pInstruments[i].op[1].iScalingOutput = f->readInt(1);
		this->pInstruments[i].op[0].iAttackDecay = f->readInt(1);
		this->pInstruments[i].op[1].iAttackDecay = f->readInt(1);
		this->pInstruments[i].op[0].iSustainRelease = f->readInt(1);
		this->pInstruments[i].op[1].iSustainRelease = f->readInt(1);
		this->pInstruments[i].op[0].iWaveSel = f->readInt(1);
		this->pInstruments[i].op[1].iWaveSel = f->readInt(1);
		this->pInstruments[i].iConnection = f->readInt(1);
		f->seek(5, binio::Add);  // skip over the padding bytes
	}

	// In the original Creative driver (SBFMDRV / fmdrv) the file's own instrument
	// table is the only one used during playback: incoming MIDI patch numbers are
	// reduced modulo the file's instrument count (g_num_inst) rather than being
	// looked up against a 128-slot table padded with default patches.  We follow
	// that behaviour here.  iInstCount is the divisor used for that wraparound (see
	// the 0xC0 program-change handler).
	if (this->cmfHeader.iNumInstruments > 0) {
		this->iInstCount = this->cmfHeader.iNumInstruments;
	} else {
		// A file with no instruments of its own falls back to the default bank
		// (this matches fmdrv's pre-song state of g_num_inst == 16).
		this->iInstCount = 16;
		for (int i = 0; i < 16; i++) {
			this->pInstruments[i].op[0].iCharMult =       cDefaultPatches[i * 11 + 0];
			this->pInstruments[i].op[1].iCharMult =       cDefaultPatches[i * 11 + 1];
			this->pInstruments[i].op[0].iScalingOutput =  cDefaultPatches[i * 11 + 2];
			this->pInstruments[i].op[1].iScalingOutput =  cDefaultPatches[i * 11 + 3];
			this->pInstruments[i].op[0].iAttackDecay =    cDefaultPatches[i * 11 + 4];
			this->pInstruments[i].op[1].iAttackDecay =    cDefaultPatches[i * 11 + 5];
			this->pInstruments[i].op[0].iSustainRelease = cDefaultPatches[i * 11 + 6];
			this->pInstruments[i].op[1].iSustainRelease = cDefaultPatches[i * 11 + 7];
			this->pInstruments[i].op[0].iWaveSel =        cDefaultPatches[i * 11 + 8];
			this->pInstruments[i].op[1].iWaveSel =        cDefaultPatches[i * 11 + 9];
			this->pInstruments[i].iConnection =           cDefaultPatches[i * 11 + 10];
		}
	}

	if (this->cmfHeader.iTagOffsetTitle) {
		f->seek(this->cmfHeader.iTagOffsetTitle);
		this->strTitle = f->readString('\0');
	}
	if (this->cmfHeader.iTagOffsetComposer) {
		f->seek(this->cmfHeader.iTagOffsetComposer);
		this->strComposer = f->readString('\0');
	}
	if (this->cmfHeader.iTagOffsetRemarks) {
		f->seek(this->cmfHeader.iTagOffsetRemarks);
		this->strRemarks = f->readString('\0');
	}

	// Load the MIDI data into memory
  f->seek(this->cmfHeader.iMusicOffset);
  this->iSongLen = fp.filesize(f) - this->cmfHeader.iMusicOffset;
  if (this->iSongLen <= 0) { // invalid offset value
    fp.close(f);
    return false;
  }
  this->data = new unsigned char[this->iSongLen];
  f->readString((char *)data, this->iSongLen);

  fp.close(f);
	rewind(0);

  return true;
}

bool CcmfPlayer::update()
{
	// This has to be here and not in getrefresh() for some reason.
	this->iDelayRemaining = 0;

	// Read in the next event
	while (!this->iDelayRemaining) {
		uint8_t iCommand = this->iPlayPointer < this->iSongLen ? this->data[this->iPlayPointer++] : 0;
		if ((iCommand & 0x80) == 0) {
			// Running status, use previous command
			this->iPlayPointer--;
			iCommand = this->iPrevCommand;
		} else {
			this->iPrevCommand = iCommand;
		}
		uint8_t iChannel = iCommand & 0x0F;
		switch (iCommand & 0xF0) {
			case 0x80: { // Note off (two data bytes)
				if (this->iPlayPointer > this->iSongLen - 2) break;
				uint8_t iNote = this->data[this->iPlayPointer++];
				uint8_t iVelocity = this->data[this->iPlayPointer++]; // release velocity
				this->cmfNoteOff(iChannel, iNote, iVelocity);
				break;
			}
			case 0x90: { // Note on (two data bytes)
				if (this->iPlayPointer > this->iSongLen - 2) break;
				uint8_t iNote = this->data[this->iPlayPointer++];
				uint8_t iVelocity = this->data[this->iPlayPointer++]; // attack velocity
				if (iVelocity) {
					if (iNotePlaying[iChannel] == iNote)
					{	// Note duplicated, turn it off
						iVelocity = 0;
						// Fix this on next Note Off event
						bNoteFix[iChannel] = true;
					}
				}
				else {
					if (bNoteFix[iChannel])
					{	// Turn on this note again
						iVelocity = 127;
						// Fix not needed anymore
						bNoteFix[iChannel] = false;
					}
				}
				// Store last played note
				iNotePlaying[iChannel] = (iVelocity ? iNote : 255);
				if (iVelocity) {
					this->cmfNoteOn(iChannel, iNote, iVelocity);
				} else {
					// This is a note-off instead (velocity == 0)
					this->cmfNoteOff(iChannel, iNote, iVelocity); // 64 is the MIDI default note-off velocity
					break;
				}
				break;
			}
			case 0xA0: { // Polyphonic key pressure (two data bytes)
				if (this->iPlayPointer > this->iSongLen - 2) break;
				uint8_t iNote = this->data[this->iPlayPointer++];
				uint8_t iPressure = this->data[this->iPlayPointer++];
				AdPlug_LogWrite("CMF: Key pressure not yet implemented! (wanted ch%d/note %d set to %d)\n", iChannel, iNote, iPressure);
				break;
			}
			case 0xB0: { // Controller (two data bytes)
				if (this->iPlayPointer > this->iSongLen - 2) break;
				uint8_t iController = this->data[this->iPlayPointer++];
				uint8_t iValue = this->data[this->iPlayPointer++];
				this->MIDIcontroller(iChannel, iController, iValue);
				break;
			}
			case 0xC0: { // Instrument change (one data byte)
				if (this->iPlayPointer >= this->iSongLen) break;
				uint8_t iNewInstrument = this->data[this->iPlayPointer++];
				// Reduce the patch number modulo the file's instrument count, as
				// the original SBFMDRV driver does (insnum %= g_num_inst).
				int iPatch = (this->iInstCount > 0) ? (iNewInstrument % this->iInstCount) : 0;
				this->chMIDI[iChannel].iPatch = iPatch;
				AdPlug_LogWrite("CMF: Remembering MIDI channel %d now uses patch %d (requested %d)\n", iChannel, iPatch, iNewInstrument);
				break;
			}
			case 0xD0: { // Channel pressure (one data byte)
				if (this->iPlayPointer >= this->iSongLen) break;
				uint8_t iPressure = this->data[this->iPlayPointer++];
				AdPlug_LogWrite("CMF: Channel pressure not yet implemented! (wanted ch%d set to %d)\n", iChannel, iPressure);
				break;
			}
			case 0xE0: { // Pitch bend (two data bytes)
				if (this->iPlayPointer > this->iSongLen - 2) break;
				uint8_t iLSB = this->data[this->iPlayPointer++];
				uint8_t iMSB = this->data[this->iPlayPointer++];
				uint16_t iValue = (iMSB << 7) | iLSB;
				// 8192 is middle/off, 0 is -2 semitones, 16384 is +2 semitones
				this->chMIDI[iChannel].iPitchbend = iValue;
				this->cmfNoteUpdate(iChannel);
				AdPlug_LogWrite("CMF: Channel %d pitchbent to %d (%+.2f)\n", iChannel + 1, iValue, (float)(iValue - 8192) / 8192);
				break;
			}
			case 0xF0: // System message (arbitrary data bytes)
				switch (iCommand) {
					case 0xF0: { // Sysex
						// SMF/CMF sysex events are variable-length-prefixed:
						// F0 <vlq length> <data...>.  Match the SBFMDRV reference
						// (viiri/fmdrv) by reading the length and skipping the
						// payload, instead of scanning for the next byte with the
						// MSB set (which desyncs if a data byte happens to be >= 0x80).
						uint32_t iSysexLen = this->readMIDINumber();
						AdPlug_LogWrite("Sysex message: %u bytes\n", iSysexLen);
						// This will have read in the terminating EOX (0xF7) message too
						if (iSysexLen > (uint32_t)(this->iSongLen - this->iPlayPointer))
							this->iPlayPointer = this->iSongLen;
						else
							this->iPlayPointer += iSysexLen;
						break;
					}
					case 0xF1: // MIDI Time Code Quarter Frame
						if (this->iPlayPointer < this->iSongLen)
							(void)this->data[this->iPlayPointer++]; // message data (ignored)
						break;
					case 0xF2: // Song position pointer
						if (this->iPlayPointer < this->iSongLen - 1) {
							(void)this->data[this->iPlayPointer++]; // message data (ignored)
							(void)this->data[this->iPlayPointer++];
						}
						break;
					case 0xF3: // Song select
						if (this->iPlayPointer < this->iSongLen - 1) {
							(void)this->data[this->iPlayPointer++]; // message data (ignored)
							AdPlug_LogWrite("CMF: MIDI Song Select is not implemented.\n");
						}
						break;
					case 0xF6: // Tune request
						break;
					case 0xF7: { // End of System Exclusive (EOX) - should never be read, should be absorbed by Sysex handling code
						// If encountered standalone as an "escape" event it is also
						// length-prefixed (F7 <vlq length> <data...>); skip the payload
						// to stay in sync (matches the fmdrv reference).
						uint32_t iEoxLen = this->readMIDINumber();
						if (iEoxLen > (uint32_t)(this->iSongLen - this->iPlayPointer))
							this->iPlayPointer = this->iSongLen;
						else
							this->iPlayPointer += iEoxLen;
						break;
					}

					// These messages are "real time", meaning they can be sent between
					// the bytes of other messages - but we're lazy and don't handle these
					// here (hopefully they're not necessary in a MIDI file, and even less
					// likely to occur in a CMF.)
					case 0xF8: // Timing clock (sent 24 times per quarter note, only when playing)
					case 0xFA: // Start
					case 0xFB: // Continue
					case 0xFE: // Active sensing (sent every 300ms or MIDI connection assumed lost)
						break;
					case 0xFC: // Stop
						AdPlug_LogWrite("CMF: Received Real Time Stop message (0xFC)\n");
						this->bSongEnd = true;
						this->iPlayPointer = 0; // for repeat in endless-play mode
						break;
					case 0xFF: { // System reset, used as meta-events in a MIDI file
						if (this->iPlayPointer >= this->iSongLen) break;
						uint8_t iEvent = this->data[this->iPlayPointer++];
						// Meta-events are length-prefixed: FF <type> <vlq length> <data...>.
						// Read the length so we can skip the payload and stay in sync on
						// any meta-event other than end-of-track (matches the fmdrv reference,
						// which previously was not consumed here and could desync playback).
						uint32_t iMetaLen = this->readMIDINumber();
						switch (iEvent) {
							case 0x2F: // end of track
								AdPlug_LogWrite("CMF: End-of-track, stopping playback\n");
								this->bSongEnd = true;
								this->iPlayPointer = 0; // for repeat in endless-play mode
								break;
							default:
								AdPlug_LogWrite("CMF: Unknown MIDI meta-event 0xFF 0x%02X\n", iEvent);
								if (iMetaLen > (uint32_t)(this->iSongLen - this->iPlayPointer))
									this->iPlayPointer = this->iSongLen;
								else
									this->iPlayPointer += iMetaLen;
								break;
						}
						break;
					}
					default:
						AdPlug_LogWrite("CMF: Unknown MIDI system command 0x%02X\n", iCommand);
						break;
				}
				break;
			default:
				AdPlug_LogWrite("CMF: Unknown MIDI command 0x%02X\n", iCommand);
				break;
		}

		if (this->iPlayPointer >= this->iSongLen) {
			this->bSongEnd = true;
			this->iPlayPointer = 0; // for repeat in endless-play mode
		}

		// Read in the number of ticks until the next event
		this->iDelayRemaining = this->readMIDINumber();
	}

	return !this->bSongEnd;
}

void CcmfPlayer::rewind(int subsong)
{
  this->opl->init();

	// Initialise

  // Enable use of WaveSel register on OPL3 (even though we're only an OPL2!)
  // Apparently this enables nine-channel mode?
	this->writeOPL(0x01, 0x20);

	// Disable OPL3 mode (can be left enabled by a previous non-CMF song)
	// (AdPlug-specific: fmdrv is a pure OPL2 driver and never touches 0x05.)
	this->writeOPL(0x05, 0x00);

	// Really make sure CSM+SEL are off (again, Creative's player...)
	this->writeOPL(0x08, 0x00);

	// Program every melodic channel with the driver's default instrument so
	// that channels used before receiving a patch change start from the same
	// register state as the real SBFMDRV driver (fmdrv's opl_reset2()).  This
	// replaces the old per-channel "hihat" frequency kludge (writes to the
	// fnum/keyon registers of channels 6-8), which was a guess that helped one
	// song and hurt another.
	// Per the fidelity policy above, the feedback/connection byte is written to
	// the correct 0xC0 + channel register, not fmdrv's buggy "opl_reg_offs[i]+i".
	for (int i = 0; i < 9; i++) {
		uint8_t iOffset = OPLOFFSET(i);
		this->writeOPL(BASE_CHAR_MULT + iOffset,     cInitInstrument[0]);
		this->writeOPL(BASE_CHAR_MULT + iOffset + 3, cInitInstrument[1]);
		this->writeOPL(BASE_SCAL_LEVL + iOffset,     cInitInstrument[2]);
		this->writeOPL(BASE_SCAL_LEVL + iOffset + 3, cInitInstrument[3]);
		this->writeOPL(BASE_ATCK_DCAY + iOffset,     cInitInstrument[4]);
		this->writeOPL(BASE_ATCK_DCAY + iOffset + 3, cInitInstrument[5]);
		this->writeOPL(BASE_SUST_RLSE + iOffset,     cInitInstrument[6]);
		this->writeOPL(BASE_SUST_RLSE + iOffset + 3, cInitInstrument[7]);
		this->writeOPL(BASE_WAVE      + iOffset,     cInitInstrument[8]);
		this->writeOPL(BASE_WAVE      + iOffset + 3, cInitInstrument[9]);
		this->writeOPL(BASE_FEED_CONN + i,           cInitInstrument[10]);
	}

	// Amplify AM + VIB depth.  Creative's CMF player does this, and there
	// doesn't seem to be any way to stop it from doing so - except for the
	// non-standard controller 0x63 I added :-)
	this->writeOPL(0xBD, 0xC0);

	this->bSongEnd = false;
	this->iPlayPointer = 0;
	this->iPrevCommand = 0; // just in case
	this->iNoteCount = 0;

	// Read in the number of ticks until the first event
	this->iDelayRemaining = this->readMIDINumber();

  // Reset song state.  This used to be in the constructor, but the XMMS2
  // plugin sets the song length before starting playback.  AdPlug plays the
  // song in its entirety (with no synth) to determine the song length, which
  // results in the state variables below matching the end of the song.  When
  // the real OPL synth is activated for playback, it no longer matches the
  // state variables and the instruments are not set correctly!
	for (int i = 0; i < 9; i++) {
		this->chOPL[i].iNoteStart = 0; // no note playing atm
		this->chOPL[i].iMIDINote = -1;
		this->chOPL[i].iMIDIChannel = -1;
		this->chOPL[i].iMIDIPatch = -1;

		this->chMIDI[i].iPatch = 0; // fmdrv default instrument is 0 (avoids OOB if a note precedes any program change)
		this->chMIDI[i].iPitchbend = 8192;
		this->chMIDI[i].iTranspose = 0;
	}
	for (int i = 9; i < 16; i++) {
		this->chMIDI[i].iPatch = 0; // fmdrv default instrument is 0
		this->chMIDI[i].iPitchbend = 8192;
		this->chMIDI[i].iTranspose = 0;
	}

	memset(this->iCurrentRegs, 0, 256);
	memset(this->iNotePlaying, 255, sizeof(iNotePlaying));
	memset(this->bNoteFix, false, sizeof(bNoteFix));

	return;
}

// Return value: 1 == 1 second, 2 == 0.5 seconds
float CcmfPlayer::getrefresh()
{
	if (this->iDelayRemaining) {
		return (float)this->cmfHeader.iTicksPerSecond / (float)this->iDelayRemaining;
	} else {
		// Delay-remaining is zero (e.g. start of song) so use a tiny delay
		return this->cmfHeader.iTicksPerSecond; // wait for one tick
	}
}

std::string CcmfPlayer::gettitle()
{
	return this->strTitle;
}
std::string CcmfPlayer::getauthor()
{
	return this->strComposer;
}
std::string CcmfPlayer::getdesc()
{
	return this->strRemarks;
}


//
// PROTECTED
//

// Read a variable-length integer from MIDI data
uint32_t CcmfPlayer::readMIDINumber()
{
	uint32_t iValue = 0;
	for (int i = 0; i < 4; i++) {
		uint8_t iNext = this->iPlayPointer < this->iSongLen ? this->data[this->iPlayPointer++] : 0;
		iValue <<= 7;
		iValue |= (iNext & 0x7F); // ignore the MSB
		if ((iNext & 0x80) == 0) break; // last byte has the MSB unset
	}
	return iValue;
}

// Note on fidelity to the original Creative SBFMDRV driver:
// This player is being aligned with the accurate SBFMDRV C port (viiri/fmdrv),
// but a couple of genuine bugs in the original driver are *deliberately not*
// reproduced here, because correct behaviour sounds better and AdPlug already
// does the right thing:
//   1. fmdrv's midi_panic() writes the fixed register 0x83 for every voice
//      (a hard-coded constant) instead of the per-voice sustain/release
//      register 0x80 + offset.  We use the correct per-voice register.
//   2. fmdrv's opl_reset2() writes the feedback/connection value to
//      "opl_reg_offs[i] + i" rather than the proper 0xC0 + channel register.
//      writeInstrumentSettings() below already uses BASE_FEED_CONN (0xC0) +
//      channel, which is correct.
// If byte-identical OPL register streams to the real driver are ever required,
// these two quirks would need to be re-introduced.

// iChannel: OPL channel (0-8)
// iOperator: 0 == Modulator, 1 == Carrier
//   Source - source operator to read from instrument definition
//   Dest - destination operator on OPL chip
// iInstrument: Index into this->pInstruments array of CMF instruments
void CcmfPlayer::writeInstrumentSettings(uint8_t iChannel, uint8_t iOperatorSource, uint8_t iOperatorDest, uint8_t iInstrument)
{
	assert(iChannel <= 8);

	uint8_t iOPLOffset = OPLOFFSET(iChannel);
	if (iOperatorDest) iOPLOffset += 3; // Carrier if iOperator == 1 (else Modulator)

	this->writeOPL(BASE_CHAR_MULT + iOPLOffset, this->pInstruments[iInstrument].op[iOperatorSource].iCharMult);
	this->writeOPL(BASE_SCAL_LEVL + iOPLOffset, this->pInstruments[iInstrument].op[iOperatorSource].iScalingOutput);
	this->writeOPL(BASE_ATCK_DCAY + iOPLOffset, this->pInstruments[iInstrument].op[iOperatorSource].iAttackDecay);
	this->writeOPL(BASE_SUST_RLSE + iOPLOffset, this->pInstruments[iInstrument].op[iOperatorSource].iSustainRelease);
	this->writeOPL(BASE_WAVE      + iOPLOffset, this->pInstruments[iInstrument].op[iOperatorSource].iWaveSel);

	// TODO: Check to see whether we should only be loading this for one or both operators
	this->writeOPL(BASE_FEED_CONN + iChannel, this->pInstruments[iInstrument].iConnection);
	return;
}

// Write a byte to the OPL "chip" and update the current record of register states
void CcmfPlayer::writeOPL(uint8_t iRegister, uint8_t iValue)
{
	this->opl->write(iRegister, iValue);
	this->iCurrentRegs[iRegister] = iValue;
	return;
}

void CcmfPlayer::getFreq(uint8_t iChannel, uint8_t iNote, uint8_t * iBlock, uint16_t * iOPLFNum)
{
	// Frequency generation ported from the SBFMDRV reference (viiri/fmdrv:
	// note2fnum() + calc_block_fnum()).  This replaces AdPlug's old pow()-based
	// approximation with the real Creative driver's lookup tables, matching its
	// tuning exactly.

	// note2fnum(): clamp the MIDI note and look up its block/note byte.
	// (fmdrv's global transpose g_transp is always 0, so it is omitted here.)
	int iClampedNote = iNote;
	if (iClampedNote < 0) iClampedNote = 0;
	if (iClampedNote > 127) iClampedNote = 127;
	uint8_t iBlockNote = block_note_tbl[iClampedNote];

	// calc_block_fnum(): split into octave (block) and a note index measured in
	// 1/64ths of a semitone (12 semitones * 64 == 768 == fnum_tbl size).
	int iBlk = (iBlockNote & 0x70) >> 4;       // octave, 0..7
	int iNoteIdx = (iBlockNote & 0x0F) << 6;   // semitone-within-octave * 64

	// Per-channel transpose.  AdPlug stores the controller value directly (1/256
	// of a semitone per unit); fmdrv works in 1/64-semitone units (value / 4).
	iNoteIdx += this->chMIDI[iChannel].iTranspose / 4;

	// Pitch bend (AdPlug extension; the Creative driver has none).  Applied on top
	// in the same 1/64-semitone units, preserving the previous +/-1 semitone range
	// and staying neutral (adding 0) at the centre value of 8192.
	iNoteIdx += (this->chMIDI[iChannel].iPitchbend - 8192) / 128;

	// Wrap the note index into the table range, shifting the octave to suit
	// (matches fmdrv's calc_block_fnum, here in whole-octave steps).
	if (iNoteIdx < 0)    { iNoteIdx += 768; iBlk -= 1; if (iBlk < 0) { iNoteIdx = 0;   iBlk = 0; } }
	if (iNoteIdx >= 768) { iNoteIdx -= 768; iBlk += 1; if (iBlk > 7) { iNoteIdx = 767; iBlk = 7; } }

	*iBlock = (uint8_t)iBlk;
	*iOPLFNum = fnum_tbl[iNoteIdx];
}

void CcmfPlayer::cmfNoteOn(uint8_t iChannel, uint8_t iNote, uint8_t iVelocity)
{
	uint8_t iBlock = 0;
	uint16_t iOPLFNum = 0;
	getFreq(iChannel, iNote, &iBlock, &iOPLFNum);
	if (iOPLFNum > 1023) AdPlug_LogWrite("CMF: This note is out of range! (send this song to malvineous@shikadi.net!)\n");

	// See if we're playing a rhythm mode percussive instrument
	if ((iChannel > 10) && (this->bPercussive)) {
		uint8_t iPercChannel = this->getPercChannel(iChannel);

		// Will have to set every time (easier) than figuring out whether the mod
		// or car needs to be changed.
		this->MIDIchangeInstrument(iPercChannel, iChannel, this->chMIDI[iChannel].iPatch);

		// Velocity -> output level, using the same formula as the Creative driver
		// (SBFMDRV / fmdrv) rather than the old sqrt approximation.  The
		// instrument's carrier scaling/level byte supplies the base level and KSL
		// bits; velocity scales the level and the KSL bits are OR'd back in.
		uint8_t iCarrierScal = this->pInstruments[this->chMIDI[iChannel].iPatch].op[1].iScalingOutput;
		uint8_t iBaseLevel = 63 - (iCarrierScal & 0x3F);
		uint8_t iKSL = iCarrierScal & 0xC0;
		uint8_t iLevel = (63 - (((iVelocity | 0x80) * iBaseLevel) >> 8)) | iKSL;

		int iOPLOffset = BASE_SCAL_LEVL + OPLOFFSET(iPercChannel);

		if (iChannel == 11) iOPLOffset += 3; // only do bassdrum carrier for volume control
		this->writeOPL(iOPLOffset, iLevel);

		// Apparently you can't set the frequency for the cymbal or hihat?
		// Vinyl requires you don't set it, Kiloblaster requires you do!
		this->writeOPL(BASE_FNUM_L + iPercChannel, iOPLFNum & 0xFF);
		this->writeOPL(BASE_KEYON_FREQ + iPercChannel, (iBlock << 2) | ((iOPLFNum >> 8) & 0x03));

		uint8_t iBit = 1 << (15 - iChannel);

		// Turn the perc instrument off if it's already playing (OPL can't do
		// polyphonic notes w/ percussion)
		if (this->iCurrentRegs[BASE_RHYTHM] & iBit) this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] & ~iBit);


		// Turn the note on
		this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] | iBit);

		this->chOPL[iPercChannel].iNoteStart = ++this->iNoteCount;
		this->chOPL[iPercChannel].iMIDIChannel = iChannel;
		this->chOPL[iPercChannel].iMIDINote = iNote;

	} else { // Non rhythm-mode or a normal instrument channel

		// Figure out which OPL channel to play this note on
		int iOPLChannel = -1;
		int iNumChannels = this->bPercussive ? 6 : 9;
		for (int i = iNumChannels - 1; i >= 0; i--) {
			// If there's no note playing on this OPL channel, use that
			if (this->chOPL[i].iNoteStart == 0) {
				iOPLChannel = i;
				// See if this channel is already set to the instrument we want.
				if (this->chOPL[i].iMIDIPatch == this->chMIDI[iChannel].iPatch) {
					// It is, so stop searching
					break;
				} // else keep searching just in case there's a better match
			}
		}
		if (iOPLChannel == -1) {
			// All channels were in use, find the one with the longest note
			iOPLChannel = 0;
			int iEarliest = this->chOPL[0].iNoteStart;
			for (int i = 1; i < iNumChannels; i++) {
				if (this->chOPL[i].iNoteStart < iEarliest) {
					// Found a channel with a note being played for longer
					iOPLChannel = i;
					iEarliest = this->chOPL[i].iNoteStart;
				}
			}
			AdPlug_LogWrite("CMF: Too many polyphonic notes, cutting note on channel %d\n", iOPLChannel);
		}

		// Now the new note should be played on iOPLChannel, but see if the instrument
		// is right first.
		if (this->chOPL[iOPLChannel].iMIDIPatch != this->chMIDI[iChannel].iPatch) {
			this->MIDIchangeInstrument(iOPLChannel, iChannel, this->chMIDI[iChannel].iPatch);
		}

		this->chOPL[iOPLChannel].iNoteStart = ++this->iNoteCount;
		this->chOPL[iOPLChannel].iMIDIChannel = iChannel;
		this->chOPL[iOPLChannel].iMIDINote = iNote;

		// Adjust the carrier output level to match the note velocity, exactly as
		// the Creative driver (SBFMDRV / fmdrv) does.  The instrument's carrier
		// scaling/level byte supplies the base attenuation (level) and the
		// key-scale-level (KSL) bits; velocity scales the level and the KSL bits
		// are then OR'd back in.  Unlike the old (disabled) AdPlug approximation
		// this is applied unconditionally, as the reference driver always honours
		// velocity.
		uint8_t iCarrierScal = this->pInstruments[this->chMIDI[iChannel].iPatch].op[1].iScalingOutput;
		uint8_t iBaseLevel = 63 - (iCarrierScal & 0x3F);
		uint8_t iKSL = iCarrierScal & 0xC0;
		uint8_t iLevel = (63 - (((iVelocity | 0x80) * iBaseLevel) >> 8)) | iKSL;
		uint8_t iOPLOffset = BASE_SCAL_LEVL + OPLOFFSET(iOPLChannel) + 3; // +3 == Carrier
		this->writeOPL(iOPLOffset, iLevel);

		// Set the frequency and play the note
		this->writeOPL(BASE_FNUM_L + iOPLChannel, iOPLFNum & 0xFF);
		this->writeOPL(BASE_KEYON_FREQ + iOPLChannel, OPLBIT_KEYON | (iBlock << 2) | ((iOPLFNum & 0x300) >> 8));
	}
	return;
}

void CcmfPlayer::cmfNoteOff(uint8_t iChannel, uint8_t iNote, uint8_t iVelocity)
{
	if ((iChannel > 10) && (this->bPercussive)) {
		int iOPLChannel = this->getPercChannel(iChannel);
		if (this->chOPL[iOPLChannel].iMIDINote != iNote) return; // there's a different note playing now
		this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] & ~(1 << (15 - iChannel)));
		this->chOPL[iOPLChannel].iNoteStart = 0; // channel free
	} else { // Non rhythm-mode or a normal instrument channel
		int iOPLChannel = -1;
		int iNumChannels = this->bPercussive ? 6 : 9;
		for (int i = 0; i < iNumChannels; i++) {
			if (
				(this->chOPL[i].iMIDIChannel == iChannel) &&
				(this->chOPL[i].iMIDINote == iNote) &&
				(this->chOPL[i].iNoteStart != 0)
			) {
				// Found the note, switch it off
				this->chOPL[i].iNoteStart = 0;
				iOPLChannel = i;
				break;
			}
		}
		if (iOPLChannel == -1) return;

		this->writeOPL(BASE_KEYON_FREQ + iOPLChannel, this->iCurrentRegs[BASE_KEYON_FREQ + iOPLChannel] & ~OPLBIT_KEYON);
	}
	return;
}

void CcmfPlayer::cmfNoteUpdate(uint8_t iChannel)
{
	uint8_t iBlock = 0;
	uint16_t iOPLFNum = 0;

	// See if we're playing a rhythm mode percussive instrument
	if ((iChannel > 10) && (this->bPercussive)) {
		uint8_t iPercChannel = this->getPercChannel(iChannel);
		getFreq(iChannel, this->chOPL[iPercChannel].iMIDINote, &iBlock, &iOPLFNum);

		// Update note frequency
		this->writeOPL(BASE_FNUM_L + iPercChannel, iOPLFNum & 0xFF);
		this->writeOPL(BASE_KEYON_FREQ + iPercChannel, (iBlock << 2) | ((iOPLFNum >> 8) & 0x03));

	} else { // Non rhythm-mode or a normal instrument channel

		// Figure out which OPL channels should be updated
		int iNumChannels = this->bPercussive ? 6 : 9;
		for (int i = 0; i < iNumChannels; i++) {
			// Needed channel and note is playing
			if (this->chOPL[i].iMIDIChannel == iChannel && this->chOPL[i].iNoteStart > 0) {
				// Update note frequency
				getFreq(iChannel, this->chOPL[i].iMIDINote, &iBlock, &iOPLFNum);
				this->writeOPL(BASE_FNUM_L + i, iOPLFNum & 0xFF);
				this->writeOPL(BASE_KEYON_FREQ + i, OPLBIT_KEYON | (iBlock << 2) | ((iOPLFNum & 0x300) >> 8));
			}
		}
	}
	return;
}

uint8_t CcmfPlayer::getPercChannel(uint8_t iChannel)
{
	switch (iChannel) {
		case 11: return 7-1; // Bass drum
		case 12: return 8-1; // Snare drum
		case 13: return 9-1; // Tom tom
		case 14: return 9-1; // Top cymbal
		case 15: return 8-1; // Hihat
	}
	AdPlug_LogWrite("CMF ERR: Tried to get the percussion channel from MIDI channel %d - this shouldn't happen!\n", iChannel);
	return 0;
}


void CcmfPlayer::MIDIchangeInstrument(uint8_t iOPLChannel, uint8_t iMIDIChannel, uint8_t iNewInstrument)
{
	if ((iMIDIChannel > 10) && (this->bPercussive)) {
		switch (iMIDIChannel) {
			case 11: // Bass drum (operator 13+16 == channel 7 modulator+carrier)
				this->writeInstrumentSettings(7-1, 0, 0, iNewInstrument);
				this->writeInstrumentSettings(7-1, 1, 1, iNewInstrument);
				break;
			case 12: // Snare drum (operator 17 == channel 8 carrier)
			//case 15:
				this->writeInstrumentSettings(8-1, 0, 1, iNewInstrument);

				//
				//this->writeInstrumentSettings(8-1, 0, 0, iNewInstrument);
				break;
			case 13: // Tom tom (operator 15 == channel 9 modulator)
			//case 14:
				this->writeInstrumentSettings(9-1, 0, 0, iNewInstrument);

				//
				//this->writeInstrumentSettings(9-1, 0, 1, iNewInstrument);
				break;
			case 14: // Top cymbal (operator 18 == channel 9 carrier)
				this->writeInstrumentSettings(9-1, 0, 1, iNewInstrument);
				break;
			case 15: // Hi-hat (operator 14 == channel 8 modulator)
				this->writeInstrumentSettings(8-1, 0, 0, iNewInstrument);
				break;
			default:
				AdPlug_LogWrite("CMF: Invalid MIDI channel %d (not melodic and not percussive!)\n", iMIDIChannel + 1);
				break;
		}
		this->chOPL[iOPLChannel].iMIDIPatch = iNewInstrument;
	} else {
		// Standard nine OPL channels
		this->writeInstrumentSettings(iOPLChannel, 0, 0, iNewInstrument);
		this->writeInstrumentSettings(iOPLChannel, 1, 1, iNewInstrument);
		this->chOPL[iOPLChannel].iMIDIPatch = iNewInstrument;
	}
	return;
}

void CcmfPlayer::MIDIcontroller(uint8_t iChannel, uint8_t iController, uint8_t iValue)
{
	switch (iController) {
		case 0x63:
			// Custom extension to allow CMF files to switch the AM+VIB depth on and
			// off (officially both are on, and there's no way to switch them off.)
			// Controller values:
			//   0 == AM+VIB off
			//   1 == VIB on
			//   2 == AM on
			//   3 == AM+VIB on
			if (iValue) {
				this->writeOPL(BASE_RHYTHM, (this->iCurrentRegs[BASE_RHYTHM] & ~0xC0) | (iValue << 6)); // switch AM+VIB extension on
			} else {
				this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] & ~0xC0); // switch AM+VIB extension off
			}
			AdPlug_LogWrite("CMF: AM+VIB depth change - AM %s, VIB %s\n",
				(this->iCurrentRegs[BASE_RHYTHM] & 0x80) ? "on" : "off",
				(this->iCurrentRegs[BASE_RHYTHM] & 0x40) ? "on" : "off");
			break;
		case 0x66:
			AdPlug_LogWrite("CMF: Song set marker to 0x%02X\n", iValue);
			break;
		case 0x67:
			this->bPercussive = (iValue != 0);
			if (this->bPercussive) {
				this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] | 0x20); // switch rhythm-mode on
			} else {
				this->writeOPL(BASE_RHYTHM, this->iCurrentRegs[BASE_RHYTHM] & ~0x20); // switch rhythm-mode off
			}
			AdPlug_LogWrite("CMF: Percussive/rhythm mode %s\n", this->bPercussive ? "enabled" : "disabled");
			break;
		case 0x68:
			this->chMIDI[iChannel].iTranspose = iValue;
			this->cmfNoteUpdate(iChannel);
			AdPlug_LogWrite("CMF: Transposing all notes up by %d * 1/128ths of a semitone on channel %d.\n", iValue, iChannel + 1);
			break;
		case 0x69:
			this->chMIDI[iChannel].iTranspose = -iValue;
			this->cmfNoteUpdate(iChannel);
			AdPlug_LogWrite("CMF: Transposing all notes down by %d * 1/128ths of a semitone on channel %d.\n", iValue, iChannel + 1);
			break;
		default:
			AdPlug_LogWrite("CMF: Unsupported MIDI controller 0x%02X, ignoring.\n", iController);
			break;
	}
	return;
}

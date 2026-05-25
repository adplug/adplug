/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2025 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * wm.cpp - MusicV WM Player
 *
 * Ported from MUSICV_2.COM, the DOS OPL3 sound driver used used in many
 * visual novel games developed and published by a japanese company named
 * Silky's (part of Elf Co., Ltd.).
 *
 */

#include <cstring>
#include "wm.h"

/*** static tables (from MUSICV_2.COM) ******************/

/* Raw phase-increment frequency table (ASM CS:0x103); the replayer
   normalises to fnum+block by shifting right until < 0x400. Index is
   clamped to 0..127, so only the first 128 entries are needed. */
static const unsigned short asm_freq_tab[128] = {
    0x0159,0x016D,0x0183,0x019A,0x01B3,0x01CC,0x01E7,0x0204,
    0x0223,0x0244,0x0266,0x028B,0x02B2,0x02DA,0x0307,0x0333,
    0x0366,0x0398,0x03CF,0x0409,0x0446,0x0487,0x04CC,0x0516,
    0x0565,0x05B4,0x060D,0x0667,0x06CB,0x072F,0x079E,0x0812,
    0x088B,0x090F,0x0998,0x0A2B,0x0AC9,0x0B68,0x0C1B,0x0CCE,
    0x0D96,0x0E5E,0x0F3C,0x1024,0x1116,0x121E,0x1330,0x1457,
    0x1593,0x16CF,0x1836,0x199C,0x1B2C,0x1CBD,0x1E78,0x2047,
    0x222C,0x243B,0x265F,0x28AE,0x2B26,0x2D9E,0x306B,0x3338,
    0x3659,0x397A,0x3CEF,0x408F,0x4458,0x4876,0x4CBF,0x515B,
    0x564C,0x5B3D,0x60D6,0x6670,0x6CB2,0x72F4,0x79DE,0x811D,
    0x88B1,0x90ED,0x997D,0xA2B6,0xAC98,0xB679,0xC1AC,0xCCDF,
    0xD963,0xE5E7,0xF3BD,0x1E60,0xEB06,
    0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,
    0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,
    0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,
    0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,0xEB00,
    0xEB00,0xEB00,0xEB00
};

/* Volume->attenuation table (ASM 0x514): vol 0 = quiet, vol 15 = loud. */
static const unsigned char vol_tab[16] = {
    0x32, 0x2d, 0x29, 0x25, 0x21, 0x1e, 0x1b, 0x18,
    0x15, 0x12, 0x0f, 0x0c, 0x09, 0x06, 0x03, 0x00
};

/* Connection-byte table (ASM 0x510), indexed by inst_flags & 3. Each byte
   is a bitmask of which of the 4 operators are carriers (volume-scaled). */
static const unsigned char conn_table[4] = { 0x08, 0x0a, 0x09, 0x0d };

/* XLAT table for the C-register computation (MUSICV_2.COM 0x4B7). */
static const unsigned char c_xlat_table[16] = {
    0x00, 0x50, 0xA0, 0xF0, 0x88, 0x44, 0x3C, 0x8B,
    0x1E, 0xE4, 0x09, 0xBA, 0x1E, 0x00, 0xB9, 0x40
};

/* Per-ASM-channel OPL3 mappings. */
static const unsigned char asm_to_nuked_ch[6] = { 0, 1, 2, 9, 10, 11 };
static const unsigned char ch_bank[6]         = { 0, 0, 0, 1, 1, 1 };
static const unsigned char ch_slot_id[6]      = { 0, 1, 2, 0, 1, 2 };

/* Operator base registers and register-group offsets. */
static const unsigned char  op_base[4] = { 0x20, 0x23, 0x28, 0x2B };
static const unsigned short reg_grp[5] = { 0x20, 0x40, 0x60, 0x80, 0xE0 };

/*** stream helpers *************************************/

static inline unsigned char rd_byte(unsigned char **p, const unsigned char *end)
{
    if (*p >= end) return 0;
    return *(*p)++;
}

static inline int rd_word(unsigned char **p, const unsigned char *end)
{
    int lo = rd_byte(p, end);
    int hi = rd_byte(p, end);
    return lo | (hi << 8);
}

static unsigned short opl_reg(int asm_ch, int op, int grp)
{
    unsigned short r = (unsigned short)(op_base[op] - 0x20 + reg_grp[grp]);
    r += ch_slot_id[asm_ch];
    if (ch_bank[asm_ch]) r |= 0x0100;
    return r;
}

/*** public methods *************************************/

CPlayer *CwmPlayer::factory(Copl *newopl)
{
    return new CwmPlayer(newopl);
}

bool CwmPlayer::load(const std::string &filename, const CFileProvider &fp)
{
    // The fixed header is exactly 0x20 bytes: a 16-byte signature followed by
    // 6 track offsets, an instrument offset and an EOF offset (all u16).
    static const unsigned long WM_HEADER_SIZE = 0x20;

    binistream *f = fp.open(filename);
    if (!f) return false;

    // Reject anything that cannot even hold the header, or that is implausibly
    // large for this 16-bit-offset format, before touching the contents.
    unsigned long fsize = fp.filesize(f);
    if (fsize < WM_HEADER_SIZE || fsize > 0x10000) { fp.close(f); return false; }

    // signature
    char sig[16];
    f->readString(sig, 16);
    if (memcmp(sig, "OPL3 DATA       ", 16)) { fp.close(f); return false; }

    // header: 6 track offsets, instrument offset, eof offset
    unsigned int rawTrack[WM_NCHANNELS];
    for (int i = 0; i < WM_NCHANNELS; i++) rawTrack[i] = f->readInt(2);
    unsigned int rawInst = f->readInt(2);
    unsigned int rawEof  = f->readInt(2);

    // Bail out if the stream went bad while reading the header.
    if (f->error()) { fp.close(f); return false; }
    size = fsize;

    // load entire file into a pristine buffer
    f->seek(0);
    delete[] data;
    data = new unsigned char[size];
    for (unsigned long i = 0; i < size; i++) data[i] = f->readInt(1);
    bool bad = f->error();
    fp.close(f);
    if (bad) { delete[] data; data = 0; size = 0; return false; }

    // Sanitise the EOF marker first: a zero or out-of-range value means "to the
    // end of the file". This is the upper bound every track is measured against.
    if (rawEof < WM_HEADER_SIZE || rawEof > size) rawEof = (unsigned int)size;
    eofOff = rawEof;

    // Clamp the instrument offset into the data region; values pointing into the
    // header (or past EOF) are treated as "no instrument table".
    if (rawInst < WM_HEADER_SIZE || rawInst >= size) rawInst = 0;
    instOff = rawInst;

    // Tracks share a flat buffer with no per-track length: each channel reads
    // from its offset through EOF and stops on an explicit 0xF0 event. Any
    // offset that points into the header region, lands past the EOF marker, or
    // runs off the buffer is rejected as unused. This is critical: computing
    // trackLen as (eofOff - offset) would underflow (unsigned) for an offset
    // beyond eofOff, yielding an enormous length and an out-of-bounds stream.
    for (int i = 0; i < WM_NCHANNELS; i++) {
        if (rawTrack[i] < WM_HEADER_SIZE || rawTrack[i] >= size ||
            rawTrack[i] >= eofOff) {
            trackOff[i] = 0;
            trackLen[i] = 0;
        } else {
            trackOff[i] = rawTrack[i];
            trackLen[i] = eofOff - rawTrack[i];
        }
    }

    // instrument table: from instOff to the nearest following boundary
    if (instOff > 0 && instOff < size) {
        unsigned long inst_end = size;
        for (int i = 0; i < WM_NCHANNELS; i++)
            if (trackOff[i] > instOff && trackOff[i] < inst_end)
                inst_end = trackOff[i];
        if (eofOff > instOff && eofOff < inst_end) inst_end = eofOff;
        instLen = inst_end - instOff;
    } else {
        instLen = 0;
    }

    rewind(-1);
    return true;
}

void CwmPlayer::resetState()
{
    // fresh mutable working copy (F2/F6 mutate loop counters in place)
    delete[] work;
    work = new unsigned char[size];
    memcpy(work, data, size);

    for (int i = 0; i < WM_NCHANNELS; i++) {
        WMChannel *ch = &chan[i];
        memset(ch, 0, sizeof(*ch));
        ch->flags = 0x80;

        // Grace-note pattern: when the first note has duration 1 the original
        // driver suppresses its spurious OPL frequency write. Pre-seed the
        // note so load_instrument's skip_setup path matches that behaviour.
        unsigned char first_dur = 0;
        unsigned char first_note = trackOff[i]
            ? scanFirstNoteDur(work + trackOff[i], trackLen[i], &first_dur) : 0;
        ch->note = (first_dur == 1) ? first_note : 0;

        ch->wait_remain = 1;
        ch->ties_factor = 4;
        ch->volume      = 11;
        ch->expression  = 7;
        ch->pan         = 7;
        ch->inst_id     = 0xFF;
        ch->c_xlat_index = 3;

        ch->stream       = trackOff[i] ? work + trackOff[i] : 0;
        ch->stream_start = ch->stream;
        ch->stream_end   = ch->stream + trackLen[i];
    }

    parseInstruments();
    transpose = 0;
    tickRate = 200;

    songEnded = false;
    tickCount = 0;

    opl->init();

    // Song-start prologue (matches DOS driver power-on):
    //   B1:04 = 0x3F  enable 4-op mode for all six channel pairs
    //   B1:05 = 0x01  OPL3 mode enable
    //   B0:BD = 0x40  tremolo/vibrato depth
    oplwrite(0x0104, 0x3F);
    oplwrite(0x0105, 0x01);
    oplwrite(0x00BD, 0x40);
}

void CwmPlayer::rewind(int)
{
    resetState();
}

float CwmPlayer::getrefresh()
{
    return (float)(tickRate ? tickRate : 200);
}

/*** OPL helpers ****************************************/

void CwmPlayer::oplwrite(int reg, int val)
{
    opl->setchip((reg >> 8) & 1);
    opl->write(reg & 0xFF, val & 0xFF);
}

void CwmPlayer::parseInstruments()
{
    delete[] insts;
    insts = 0;
    numInsts = 0;
    if (instLen < 30) return;

    int max_entries = (int)(instLen / 30);
    if (max_entries > 64) max_entries = 64;
    insts = new WMInst[max_entries];

    int n = 0;
    for (int i = 0; i < max_entries; i++) {
        const unsigned char *e = data + instOff + (unsigned long)(i * 30);
        memset(&insts[n], 0, sizeof(WMInst));
        insts[n].id    = e[0x15];
        insts[n].flags = e[0x14];
        memcpy(insts[n].regs, e, WM_INST_REGS);
        insts[n].vib_rate  = e[0x16] & 0x03;
        insts[n].vib_speed = e[0x17];
        insts[n].vib_amp   = (unsigned short)(e[0x18] | ((unsigned short)e[0x19] << 8));
        insts[n].vib_delay = e[0x1A];
        insts[n].lfo_lo    = e[0x1B];
        insts[n].lfo_hi    = e[0x1C];
        insts[n].lfo_amp   = e[0x1D];
        n++;
    }
    numInsts = n;
}

void CwmPlayer::loadInstrument(WMChannel *ch, int asm_ch, unsigned char inst_id)
{
    const unsigned char *regs = 0;
    unsigned char flags = 0;
    WMInst *inst = 0;
    for (int i = 0; i < numInsts; i++) {
        if (insts[i].id == inst_id) {
            regs  = insts[i].regs;
            flags = insts[i].flags;
            inst  = &insts[i];
            break;
        }
    }
    if (!regs) return;

    ch->inst_id     = inst_id;
    ch->inst_flags  = flags;
    ch->c_val_saved = flags;

    // Pre-compute carrier attenuation, applied inline (ASM 0x4D2).
    unsigned char conn = conn_table[flags & 3];
    int atten = (int)vol_tab[ch->volume & 0x0F] - (int)(signed char)ch->lfo_accum;
    if (atten < 0)  atten = 0;
    if (atten > 63) atten = 63;

    for (int op = 0; op < 4; op++) {
        unsigned char carrier_bit = (conn >> op) & 1;
        for (int grp = 0; grp < 5; grp++) {
            int byte_idx = op * 5 + grp;
            unsigned char val = regs[byte_idx];
            unsigned short reg = opl_reg(asm_ch, op, grp);
            if (grp == 1 && carrier_bit) {
                int new_tl = (val & 0x3F) + atten;
                if (new_tl > 63) new_tl = 63;
                val = (val & 0xC0) | (unsigned char)new_tl;
            }
            // Waveform select (0xE0): skip writes that don't change the
            // per-operator shadow, matching the driver's optimisation.
            if (grp == 4) {
                if (val == ch->op_waveform[op]) continue;
                ch->op_waveform[op] = val;
            }
            oplwrite(reg, val);
        }
        ch->op_tl[op] = regs[op * 5 + 1];
    }
    applyChannelC(ch, asm_ch);

    // Vibrato (freq-mod) preset (ASM 0x604).
    if (inst->vib_speed != 0) {
        ch->portamento_rate  = inst->vib_rate;
        ch->portamento_speed = inst->vib_speed;
        ch->vibrato_amp      = (signed short)inst->vib_amp;
        ch->portamento_delay = inst->vib_delay;
        portamentoInit(ch);
    } else {
        ch->effects_flags &= ~0x02;
    }

    // LFO (amp-mod / tremolo) preset (ASM 0x627).
    if (inst->lfo_lo != 0 && inst->lfo_hi != 0) {
        ch->lfo_lo  = inst->lfo_lo;
        ch->lfo_hi  = inst->lfo_hi;
        ch->lfo_amp = inst->lfo_amp;
        // lfo_setup (ASM 0x3D4): compute step and sub-step.
        ch->effects_flags |= 0x04;
        ch->lfo_sub_step = 0;
        unsigned char lo = ch->lfo_lo, hi = ch->lfo_hi;
        if (hi >= lo) {
            ch->lfo_hi = (lo > 0) ? (hi / lo) : hi;
        } else {
            ch->lfo_sub_step = (hi > 0) ? (lo / hi) : 0;
            ch->lfo_hi = 1;
        }
    } else {
        ch->effects_flags &= ~0x04;
    }

    ch->flags |= 0x08;
}

void CwmPlayer::applyChannelC(WMChannel *ch, int asm_ch)
{
    unsigned char bank  = ch_bank[asm_ch];
    unsigned char slot  = ch_slot_id[asm_ch];
    unsigned char flags = ch->inst_flags;

    unsigned short c_reg1 = (bank << 8) | (0xC0 + slot);
    unsigned short c_reg2 = (bank << 8) | (0xC3 + slot);

    // ASM formula at 0x64B.
    unsigned char translated = c_xlat_table[ch->c_xlat_index & 0x0F];
    unsigned char cl_lo  = (flags >> 1);
    unsigned char cl     = cl_lo | translated;
    unsigned char ch_bit = (flags & 1);
    unsigned char c0_val = cl;
    unsigned char c3_val = (cl & 0xFE) | ch_bit;

    oplwrite(c_reg1, c0_val);
    oplwrite(c_reg2, c3_val);
}

void CwmPlayer::applyVolume(WMChannel *ch, int asm_ch)
{
    // ASM 0x3FE: attenuation = vol_tab[volume] - lfo_accum, clamped 0..63.
    int atten = (int)vol_tab[ch->volume & 0x0F] - (int)(signed char)ch->lfo_accum;
    if (atten < 0)  atten = 0;
    if (atten > 63) atten = 63;

    unsigned char conn = conn_table[ch->c_val_saved & 3];
    for (int op = 0; op < 4; op++) {
        int bit = conn & 1;
        conn >>= 1;
        if (!bit) continue;
        unsigned short reg = opl_reg(asm_ch, op, 1);
        unsigned char tl_val = ch->op_tl[op];
        int new_tl = (tl_val & 0x3F) + atten;
        if (new_tl > 63) new_tl = 63;
        oplwrite(reg, (tl_val & 0xC0) | (unsigned char)new_tl);
    }
}

void CwmPlayer::channelNoteOff(WMChannel *ch, int asm_ch)
{
    if (!(ch->flags & 0x01)) return;
    ch->flags &= ~(0x01 | 0x02 | 0x04);
    unsigned char bank     = ch_bank[asm_ch];
    unsigned char nuked_ch = asm_to_nuked_ch[asm_ch];
    unsigned char in_bank  = nuked_ch % 9;
    unsigned short a_reg = 0xA0 + in_bank;
    unsigned short b_reg = 0xB0 + in_bank;
    if (bank) { a_reg |= 0x0100; b_reg |= 0x0100; }
    oplwrite(a_reg, ch->a_reg_val);
    oplwrite(b_reg, ch->b_reg_val);   // key-on bit not set -> note off
}

void CwmPlayer::calcFrequency(WMChannel *ch)
{
    int idx = (int)ch->note + transpose;
    if (idx < 0)   idx = 0;
    if (idx > 127) idx = 127;
    ch->freq_raw = asm_freq_tab[idx];
    ch->flags |= 0x02;
}

void CwmPlayer::applyFrequency(WMChannel *ch, int asm_ch, int freq, int write_b)
{
    unsigned char nuked_ch = asm_to_nuked_ch[asm_ch];
    unsigned char bank     = ch_bank[asm_ch];

    int octave = 0;
    while (freq > 0x3FF) { octave++; freq >>= 1; }
    unsigned short bx = (unsigned short)freq;
    unsigned char in_bank = nuked_ch % 9;
    unsigned short a_reg = 0xA0 + in_bank;
    unsigned short b_reg = 0xB0 + in_bank;
    if (bank) { a_reg |= 0x0100; b_reg |= 0x0100; }

    unsigned char new_a = (unsigned char)(bx & 0xFF);
    oplwrite(a_reg, new_a);
    ch->a_reg_val = new_a;

    if (write_b) {
        unsigned char al = (unsigned char)((bx >> 8) & 0x03) | (unsigned char)((octave & 7) << 2);
        ch->b_reg_val = al;
        if (ch->flags & 0x01) al |= 0x20;   // key-on
        oplwrite(b_reg, al);
    }
}

/*** portamento / vibrato ******************************/

void CwmPlayer::portamentoInit(WMChannel *ch)
{
    unsigned char speed = ch->portamento_speed;
    if (speed == 0) return;
    ch->effects_flags |= 0x02;
    ch->portamento_wait_ctr = speed;
    ch->portamento_flip_ctr = 0;
    signed short amp = ch->vibrato_amp;
    unsigned char rate = ch->portamento_rate;
    if (rate == 1) {
        ch->portamento_step_saved = amp;
    } else {
        ch->portamento_step_saved = (signed short)((int)amp / speed);
        ch->portamento_target     = (signed short)((int)amp % speed);
        if (rate >= 2) ch->vibrato_amp = 0;
    }
}

static void portamento_restart(CwmPlayer::WMChannel *ch);
static signed short portamento_tick(CwmPlayer::WMChannel *ch);
static void lfo_restart(CwmPlayer::WMChannel *ch);

/*** LFO ************************************************/

void CwmPlayer::lfoTick(WMChannel *ch, int asm_ch)
{
    if (!(ch->effects_flags & 0x04)) return;

    if (ch->lfo_delay_ctr > 0) {
        ch->lfo_delay_ctr--;
        if (ch->lfo_delay_ctr > 0) return;
    }

    ch->lfo_period_ctr--;
    if (ch->lfo_period_ctr == 0) {
        ch->lfo_sub_ctr    = ch->lfo_sub_step;
        ch->lfo_period_ctr = ch->lfo_lo;
        ch->lfo_step       = (unsigned char)(-(signed char)ch->lfo_step);
        ch->lfo_accum      = (unsigned char)((signed char)ch->lfo_accum + (signed char)ch->lfo_step);
        applyVolume(ch, asm_ch);
        return;
    }

    if (ch->lfo_sub_ctr > 0) {
        ch->lfo_sub_ctr--;
        if (ch->lfo_sub_ctr > 0) return;
    }
    ch->lfo_sub_ctr = ch->lfo_sub_step;
    ch->lfo_accum   = (unsigned char)((signed char)ch->lfo_accum + (signed char)ch->lfo_step);
    applyVolume(ch, asm_ch);
}

/*** per-tick frequency update *************************/

void CwmPlayer::frequencyUpdate(WMChannel *ch, int asm_ch)
{
    if (!(ch->flags & 0x80)) return;
    int freq = 0;
    int write_b = 0;
    if (ch->flags & 0x02) {
        write_b = 1;
        ch->flags &= ~0x02;
        ch->flags |= 0x01;
        ch->slide_accum = 0;
        portamento_restart(ch);
        lfo_restart(ch);
    } else if (!(ch->flags & 0x01)) {
        return;
    } else {
        if (ch->effects_flags & 0x04) lfoTick(ch, asm_ch);
        if (ch->effects_flags & 0x02) freq += portamento_tick(ch);
        if (ch->effects_flags & 0x01) {
            if (ch->wait_remain <= 1) {
                ch->freq_raw = (unsigned short)ch->slide_target;
                ch->effects_flags &= ~0x01;
            } else {
                ch->slide_accum += ch->slide_step;
                freq += ch->slide_accum;
            }
        }
    }
    freq += (int)ch->freq_raw;
    freq += (int)ch->pitch_wheel;
    freq = (int)(unsigned short)(signed short)freq;
    applyFrequency(ch, asm_ch, freq, write_b);
}

/*** loop / stack primitives ****************************/

static void portamento_restart(CwmPlayer::WMChannel *ch)
{
    if (!(ch->effects_flags & 0x02)) return;
    ch->portamento_flip_ctr  = 0;
    ch->portamento_wait_ctr  = ch->portamento_speed;
    ch->portamento_step      = ch->portamento_step_saved;
    ch->portamento_step2     = ch->portamento_target;
    ch->portamento_delay_ctr = ch->portamento_delay;
    ch->portamento_accum     = ch->vibrato_amp;
}

static signed short portamento_tick(CwmPlayer::WMChannel *ch)
{
    if (ch->portamento_delay_ctr > 0) {
        ch->portamento_delay_ctr--;
        if (ch->portamento_delay_ctr > 0) return 0;
    }

    if (ch->portamento_rate == 0) {
        ch->portamento_wait_ctr--;
        if (ch->portamento_wait_ctr != 0) {
            ch->portamento_accum -= ch->portamento_step;
            return ch->portamento_accum;
        }
        ch->portamento_wait_ctr = ch->portamento_speed;
        ch->portamento_accum    = ch->vibrato_amp;
        return ch->portamento_accum;
    }

    if (ch->portamento_rate == 1) {
        ch->portamento_wait_ctr--;
        if (ch->portamento_wait_ctr == 0) {
            ch->portamento_wait_ctr = ch->portamento_speed;
            ch->vibrato_amp = -ch->vibrato_amp;
        }
        return ch->vibrato_amp;
    }

    // rate >= 2: flip-counter / accumulator model
    ch->portamento_wait_ctr--;
    if (ch->portamento_wait_ctr != 0) {
        ch->portamento_accum += ch->portamento_step;
        return ch->portamento_accum;
    }
    ch->portamento_wait_ctr = ch->portamento_speed;
    ch->portamento_flip_ctr = (ch->portamento_flip_ctr + 1) & 3;
    if ((ch->portamento_flip_ctr & 1) == 0) {
        signed short old_accum = ch->portamento_accum;
        ch->portamento_accum = 0;
        return old_accum;
    }
    ch->portamento_accum += ch->portamento_step + ch->portamento_step2;
    ch->portamento_step  = -ch->portamento_step;
    ch->portamento_step2 = -ch->portamento_step2;
    return ch->portamento_accum;
}

static void lfo_restart(CwmPlayer::WMChannel *ch)
{
    if (!(ch->effects_flags & 0x04)) return;
    ch->lfo_period_ctr = ch->lfo_lo;
    ch->lfo_delay_ctr  = ch->lfo_amp;
    ch->lfo_sub_ctr    = ch->lfo_sub_step;
    ch->lfo_step       = ch->lfo_hi;
    ch->lfo_accum      = 0;
}

/* F1: relative jump by signed 16-bit word at current ptr (ADD DI,[DI]). */
static void loop_start(CwmPlayer::WMChannel *ch, unsigned char **ptr)
{
    unsigned char *cur = *ptr;
    if (cur + 2 <= ch->stream_end) {
        signed short off = (signed short)((unsigned short)cur[0] | ((unsigned short)cur[1] << 8));
        unsigned char *tgt = cur + off;
        if (tgt >= ch->stream_start && tgt < ch->stream_end) {
            // F1 is an unconditional jump; a backward target is the channel's
            // master loop point (its musical sequence restarts here forever).
            if (tgt < cur) ch->looped = true;
            *ptr = tgt;
        }
    }
}

/* F2: counted repeat - decrement in-stream counter, jump while > 0. */
static void loop_repeat(CwmPlayer::WMChannel *ch, unsigned char **ptr, const unsigned char *end)
{
    unsigned char *count_ptr = *ptr;
    unsigned char count = rd_byte(ptr, end);
    signed short off = (signed short)rd_word(ptr, end);
    if (count > 0) {
        unsigned char new_count = count - 1;
        count_ptr[0] = new_count;
        if (new_count > 0) {
            unsigned char *tgt = *ptr + off;
            if (tgt >= ch->stream_start && tgt < ch->stream_end) *ptr = tgt;
        }
    }
}

/* F4: push one byte onto the parameter/counter stack. */
static void f4_push(CwmPlayer::WMChannel *ch, unsigned char **ptr, const unsigned char *end)
{
    unsigned char val = rd_byte(ptr, end);
    if (ch->f4_depth < 15) {
        ch->f4_params[ch->f4_depth * 2] = val;
        ch->f4_depth++;
    }
}

/* F5: if stack top == 1, jump by signed word and pop; else skip the word. */
static void f5_skip(CwmPlayer::WMChannel *ch, unsigned char **ptr)
{
    if (ch->f4_depth > 0) {
        unsigned char val = ch->f4_params[(ch->f4_depth - 1) * 2];
        if (val == 1) {
            unsigned char *cur = *ptr;
            if (cur + 2 <= ch->stream_end) {
                signed short off = (signed short)((unsigned short)cur[0] | ((unsigned short)cur[1] << 8));
                unsigned char *tgt = cur + off;
                if (tgt >= ch->stream_start && tgt < ch->stream_end) *ptr = tgt;
            }
            ch->f4_depth--;
        } else {
            *ptr += 2;
        }
    }
}

/* F6: counted loop - decrement stack top in memory; jump while > 0, else pop. */
static void f6_loop(CwmPlayer::WMChannel *ch, unsigned char **ptr)
{
    if (ch->f4_depth > 0) {
        int idx = (ch->f4_depth - 1) * 2;
        unsigned char *val = &ch->f4_params[idx];
        (*val)--;
        if (*val == 0) {
            *ptr += 2;
            ch->f4_depth--;
        } else {
            unsigned char *cur = *ptr;
            if (cur + 2 <= ch->stream_end) {
                signed short off = (signed short)((unsigned short)cur[0] | ((unsigned short)cur[1] << 8));
                unsigned char *tgt = cur + off;
                if (tgt >= ch->stream_start && tgt < ch->stream_end) *ptr = tgt;
            }
        }
    }
}

/* F7: one-shot conditional - if stack top == 1, backward byte jump and pop. */
static void loop_end(CwmPlayer::WMChannel *ch, unsigned char **ptr)
{
    if (ch->f4_depth > 0) {
        int idx = (ch->f4_depth - 1) * 2;
        unsigned char val = ch->f4_params[idx];   // register-only decrement
        if ((unsigned char)(val - 1) == 0) {
            signed char off = (signed char)(**ptr);
            *ptr = *ptr + off;
            ch->f4_depth--;
        } else {
            (*ptr)++;
        }
    }
}

/* F8: decrement stack top; backward byte jump while > 0, else pop. */
static void f8_loop(CwmPlayer::WMChannel *ch, unsigned char **ptr)
{
    if (ch->f4_depth > 0) {
        int idx = (ch->f4_depth - 1) * 2;
        unsigned char *val = &ch->f4_params[idx];
        if (*val > 0) (*val)--;
        if (*val == 0) {
            (*ptr)++;
            ch->f4_depth--;
        } else {
            signed char off = (signed char)(**ptr);
            *ptr = *ptr + off;
        }
    }
}

/*** first-note scan (grace-note detection) *************/

unsigned char CwmPlayer::scanFirstNoteDur(const unsigned char *stream,
                                          unsigned long len, unsigned char *out_dur)
{
    const unsigned char *p = stream;
    const unsigned char *end = stream + len;
    *out_dur = 0;
    while (p < end) {
        unsigned char b = *p++;
        if (b < 0x80) {
            if (p < end) *out_dur = *p;
            return b;
        }
        if (b == 0xF0) break;
        if (b == 0x80) { p++; break; }
        if (b >= 0x81 && b <= 0x8F) {
            static const unsigned char args81[16] =
                { 0,1,0,0,2,1,0,1,1,1,1,2,0,0,0,0 };
            p += args81[b & 0x0F];
            if (p > end) break;
            continue;
        }
        if (b >= 0x90 && b <= 0x9F) {
            static const unsigned char args90[16] =
                { 3,4,0,0,1,1,0,0,0,0,0,0,0,0,0,0 };
            p += args90[b & 0x0F];
            if (p > end) break;
            continue;
        }
        if (b <= 0xEF) continue;
        switch (b) {
            case 0xF1: p += 2; break;
            case 0xF3: p += 3; break;
            case 0xF4: p += 1; break;
            case 0xF5: p += 2; break;
            case 0xF6: p += 2; break;
            case 0xF7: p += 1; break;
            case 0xF8: p += 1; break;
            case 0xF9: p += 2; break;
            default:   break;
        }
        if (p > end) break;
    }
    return 0;
}

/*** per-channel command processing *********************/

int CwmPlayer::processChannel(WMChannel *ch, int asm_ch)
{
    if (!(ch->flags & 0x80)) return 0;

    // ASM 0xB4C: note end -> key off
    if ((ch->flags & 0x01) && !(ch->flags & 0x04) &&
        ch->wait_remain == ch->ties_remain) {
        channelNoteOff(ch, asm_ch);
    }

    // ASM 0xB63: still waiting?
    if (ch->wait_remain > 0) {
        ch->wait_remain--;
        if (ch->wait_remain > 0) return 1;
    }

    // ASM 0xB6A: wait expired
    if (ch->flags & 0x04) {
        ch->effects_flags |= 0x08;
        ch->flags |= 0x08;
        ch->flags &= ~0x04;
    } else {
        ch->flags &= ~0x08;
        channelNoteOff(ch, asm_ch);
        ch->effects_flags &= ~0x01;
    }

    unsigned char **ptr = &ch->stream;
    const unsigned char *end = ch->stream_end;

    for (;;) {
        if (*ptr >= end) {
            ch->flags &= ~0x80;
            return 0;
        }
        unsigned char cmd = rd_byte(ptr, end);

        if (cmd < 0x80) {
            // note-on: note=cmd, next byte = duration
            if (cmd != ch->note) {
                ch->effects_flags &= ~0x08;
                ch->flags &= ~0x08;
            }
            ch->note = cmd;
            unsigned char dur = rd_byte(ptr, end);
            ch->wait_remain = dur;
            if (!(ch->flags & 0x04)) {
                if (ch->ties_factor == 0) {
                    ch->ties_remain = 1;
                } else {
                    int ties = (int)dur - ((int)dur * (int)ch->ties_factor / 8);
                    if (ties < 1) ties = 1;
                    ch->ties_remain = (unsigned char)ties;
                }
            }
            if (ch->flags & 0x08) return 1;   // tie / grace: keep playing
            calcFrequency(ch);
            ch->flags |= 0x01;
            return 1;
        }

        if (cmd == 0x80) {
            ch->wait_remain = rd_byte(ptr, end);
            ch->flags &= ~0x07;
            return 1;
        }

        if (cmd == 0xF0) {
            ch->flags &= ~0x80;
            channelNoteOff(ch, asm_ch);
            return 0;
        }

        if (cmd <= 0x8F) {
            switch (cmd & 0x0F) {
            case 0x01: ch->inst_id = rd_byte(ptr, end); ch->flags &= ~0x08;
                       loadInstrument(ch, asm_ch, ch->inst_id);              break;
            case 0x02:                                                       break;
            case 0x03: ch->flags |= 0x04;                                    break;
            case 0x04: {   // set tempo -> tick rate (ASM recalc_tempo 0x084F)
                int v = rd_byte(ptr, end) | (rd_byte(ptr, end) << 8);
                if (v > 0) {
                    unsigned long pit_div = 1493043UL / (unsigned long)v;
                    if (pit_div == 0) pit_div = 1;
                    tickRate = (unsigned int)(1193180UL / pit_div);
                }
                break;
            }
            case 0x05: ch->ties_factor = rd_byte(ptr, end) & 0x07;           break;
            case 0x06:                                                       break;
            case 0x07: ch->volume = rd_byte(ptr, end) & 0x0F;
                       applyVolume(ch, asm_ch);                              break;
            case 0x08: transpose = (signed char)rd_byte(ptr, end);           break;
            case 0x09: ch->c_xlat_index = rd_byte(ptr, end);
                       applyChannelC(ch, asm_ch);                            break;
            case 0x0A: ch->expression = (unsigned char)(rd_byte(ptr, end) - 1);
                       applyVolume(ch, asm_ch);                              break;
            case 0x0B: ch->pitch_wheel = (signed short)rd_word(ptr, end);    break;
            case 0x0C: if (ch->expression < 7) ch->expression++;
                       applyVolume(ch, asm_ch);                              break;
            case 0x0D: if (ch->expression > 0) ch->expression--;
                       applyVolume(ch, asm_ch);                              break;
            case 0x0E: if (ch->volume < 15) ch->volume++;
                       applyVolume(ch, asm_ch);                              break;
            case 0x0F: if (ch->volume > 0) ch->volume--;
                       applyVolume(ch, asm_ch);                              break;
            default:                                                         break;
            }
            continue;
        }

        if (cmd <= 0x9F) {
            switch (cmd) {
            case 0x90: {   // portamento/slide note-on
                int target_word = rd_word(ptr, end);
                unsigned char dur = rd_byte(ptr, end);
                int note_idx   = (target_word & 0xFF) + transpose;
                int target_idx = ((target_word >> 8) & 0xFF) + transpose;
                if (note_idx   < 0) note_idx = 0;   if (note_idx   > 127) note_idx = 127;
                if (target_idx < 0) target_idx = 0; if (target_idx > 127) target_idx = 127;
                unsigned short freq_note   = asm_freq_tab[note_idx];
                unsigned short freq_target = asm_freq_tab[target_idx];

                ch->wait_remain  = dur;
                ch->ties_remain  = 0;
                ch->freq_raw     = freq_note;
                ch->slide_target = (signed short)freq_target;

                int diff    = (int)freq_target - (int)freq_note;
                int divisor = (int)dur < 1 ? 1 : (int)dur;
                ch->slide_step  = (signed short)(diff / divisor);
                ch->slide_accum = 0;

                ch->effects_flags = (ch->effects_flags | 0x01) & ~0x08;
                ch->flags = (ch->flags & ~0x01) | 0x02;
                return 1;
            }
            case 0x91:
                ch->portamento_rate  = rd_byte(ptr, end);
                ch->portamento_speed = rd_byte(ptr, end);
                ch->vibrato_amp      = (signed short)rd_word(ptr, end);
                portamentoInit(ch);
                break;
            case 0x92: ch->effects_flags |= 0x02;                            break;
            case 0x93: ch->effects_flags &= ~0x02;                           break;
            case 0x94: ch->portamento_delay = rd_byte(ptr, end);             break;
            case 0x95: rd_byte(ptr, end);                                    break;
            default:                                                         break;
            }
            continue;
        }

        if (cmd <= 0xEF) continue;   // 0xA0-0xEF: no-ops

        switch (cmd) {
        case 0xF1: loop_start(ch, ptr);                       break;
        case 0xF2: loop_repeat(ch, ptr, end);                 break;
        case 0xF3: rd_byte(ptr, end); rd_word(ptr, end);      break;
        case 0xF4: f4_push(ch, ptr, end);                     break;
        case 0xF5: f5_skip(ch, ptr);                          break;
        case 0xF6: f6_loop(ch, ptr);                          break;
        case 0xF7: loop_end(ch, ptr);                         break;
        case 0xF8: f8_loop(ch, ptr);                          break;
        case 0xF9: { unsigned char r = rd_byte(ptr, end);
                     unsigned char v = rd_byte(ptr, end);
                     oplwrite(r, v); }                        break;
        default:                                              break;
        }
    }
}

/*** main tick ******************************************/

bool CwmPlayer::update()
{
    if (songEnded) return false;

    bool any = false;
    for (int i = 0; i < WM_NCHANNELS; i++) {
        WMChannel *ch = &chan[i];
        if (ch->flags & 0x80) {
            if (processChannel(ch, i)) any = true;
            frequencyUpdate(ch, i);
        }
    }

    // All channels reached their end-of-track (0xF0): song is finished.
    if (!any) { songEnded = true; return false; }

    // WM songs loop forever via backward jumps. Report end once every still-
    // active channel has rewound to its loop point (the master loop) at least
    // once, so songlength() terminates after one full pass. A generous tick
    // cap remains as a safety backstop for data that loops elsewhere.
    bool allLooped = true;
    for (int i = 0; i < WM_NCHANNELS; i++)
        if ((chan[i].flags & 0x80) && !chan[i].looped) { allLooped = false; break; }

    if (allLooped || ++tickCount > 2000000UL) {
        songEnded = true;
        return false;
    }
    return true;
}

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
 * wm.h - MusicV WM Player, ported from MUSICV_2.COM
 *
 * Ported from MUSICV_2.COM, the DOS OPL3 sound driver used used in many
 * visual novel games developed and published by a japanese company named
 * Silky's (part of Elf Co., Ltd.).
 *
 */

#ifndef H_ADPLUG_WM
#define H_ADPLUG_WM

#include "player.h"

class CwmPlayer: public CPlayer
{
public:
    static CPlayer *factory(Copl *newopl);

    CwmPlayer(Copl *newopl)
        : CPlayer(newopl), data(0), work(0), insts(0), numInsts(0)
    { }

    ~CwmPlayer()
    {
        delete[] data;
        delete[] work;
        delete[] insts;
    }

    bool load(const std::string &filename, const CFileProvider &fp);
    bool update();
    void rewind(int subsong = -1);
    float getrefresh();
    std::string gettype() { return std::string("MusicV WM"); }

    static const int WM_NCHANNELS = 6;
    static const int WM_INST_REGS = 20;

    // Public so the file-local stream/loop helpers in wm.cpp can operate on it.
    struct WMChannel {
        unsigned char  flags;        // bit7=active bit3=skip_setup bit2=skip_ties bit1=key_on bit0=note_on
        unsigned char  note;
        unsigned char  wait_remain;
        unsigned char  ties_remain;
        unsigned char  ties_factor;

        unsigned char  f4_depth;
        unsigned char  f4_params[16];

        unsigned char  volume;       // 0-15
        unsigned char  expression;   // 0-7
        unsigned char  pan;          // 0-15

        unsigned char  inst_id;
        unsigned char  inst_flags;
        unsigned char  op_tl[4];
        unsigned char  op_waveform[4];

        // command stream (points into the mutable work buffer)
        unsigned char *stream;
        unsigned char *stream_start;
        unsigned char *stream_end;

        unsigned short freq_raw;
        signed short   pitch_wheel;
        unsigned char  b_reg_val;
        unsigned char  a_reg_val;
        unsigned char  effects_flags; // bit2=LFO bit1=vibrato bit0=slide

        // portamento / vibrato (frequency-accumulator model)
        unsigned char  portamento_rate;
        unsigned char  portamento_flip_ctr;
        unsigned char  portamento_delay;
        unsigned char  portamento_delay_ctr;
        unsigned char  portamento_wait_ctr;
        unsigned char  portamento_speed;
        signed short   portamento_step;
        signed short   portamento_step2;
        signed short   portamento_step_saved;
        signed short   portamento_target;
        signed short   portamento_accum;
        signed short   vibrato_amp;

        // slide effect (cmd 0x90)
        signed short   slide_accum;
        signed short   slide_step;
        signed short   slide_target;

        // C register computation
        unsigned char  c_val_saved;
        unsigned char  c_xlat_index;

        // LFO (amplitude modulation / tremolo)
        unsigned char  lfo_lo;
        unsigned char  lfo_hi;
        unsigned char  lfo_amp;
        unsigned char  lfo_sub_step;
        unsigned char  lfo_delay_ctr;
        unsigned char  lfo_period_ctr;
        unsigned char  lfo_step;
        unsigned char  lfo_accum;
        unsigned char  lfo_sub_ctr;

        bool           looped;       // set once this channel rewinds to its start
    };

    struct WMInst {
        unsigned char  id;
        unsigned char  flags;
        unsigned char  regs[WM_INST_REGS];
        unsigned char  vib_rate;
        unsigned char  vib_speed;
        unsigned short vib_amp;
        unsigned char  vib_delay;
        unsigned char  lfo_lo;
        unsigned char  lfo_hi;
        unsigned char  lfo_amp;
    };

protected:
    // file image
    unsigned char *data;   // pristine copy of the loaded file
    unsigned char *work;   // mutable working copy (F2/F6 counters mutate in place)
    unsigned long  size;
    unsigned long  trackOff[WM_NCHANNELS];
    unsigned long  trackLen[WM_NCHANNELS];
    unsigned long  instOff;
    unsigned long  instLen;
    unsigned long  eofOff;

    // playback state
    WMChannel      chan[WM_NCHANNELS];
    WMInst        *insts;
    int            numInsts;
    signed char    transpose;
    unsigned int   tickRate;

    // loop detection: WM songs repeat indefinitely, so song-end is reported
    // once every active channel has rewound to its loop point (master loop).
    // A tick cap remains as a safety backstop for pathological data.
    bool                              songEnded;
    unsigned long                     tickCount;

    // helpers
    void resetState();
    void oplwrite(int reg, int val);
    void parseInstruments();
    void loadInstrument(WMChannel *ch, int asmch, unsigned char id);
    void applyChannelC(WMChannel *ch, int asmch);
    void applyVolume(WMChannel *ch, int asmch);
    void channelNoteOff(WMChannel *ch, int asmch);
    void calcFrequency(WMChannel *ch);
    void applyFrequency(WMChannel *ch, int asmch, int di, int write_b);
    void portamentoInit(WMChannel *ch);
    void lfoTick(WMChannel *ch, int asmch);
    void frequencyUpdate(WMChannel *ch, int asmch);
    int  processChannel(WMChannel *ch, int asmch);
    unsigned char scanFirstNoteDur(const unsigned char *stream,
                                   unsigned long len, unsigned char *out_dur);
};

#endif

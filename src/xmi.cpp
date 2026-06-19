/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2024 Simon Peter, <dn.tlp@gmx.net>, et al.
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
 * xmi.cpp - XMIDI (Extended MIDI) player for AdPlug
 *
 * Based on the Miles Design XMIDI32 library (XMIDI32.ASM / YAMAHA32.INC),
 * originally written in 80386 assembly by:
 *   John Miles and John Lemberger, Miles Design, Inc. (1992-1993)
 *   Copyright (C) 1991, 1992 John Miles and Miles Design, Inc.
 * 
 * REFERENCE: http://www.thegleam.com/ke5fx/misc/AIL2.ZIP
 *
 * The XMIDI standard was developed in collaboration with Origin Systems, Inc.
 * Special acknowledgement to: Martin Galway, Dana Glover, Chris Roberts,
 *   Marc Schaefgen, Gary Scott Smith, Nenad Vugrinec, and Kirk Winterrowd.
 */

#include <string.h>
#include <stdio.h>
#include "xmi.h"
#include "xmi_timbres.h"

#define DEF_PITCH_RANGE 12
#define DEF_PITCH_L 0x00
#define DEF_PITCH_H 0x40

#define U_ALL_REGS   0xF9
#define U_AVEKM      0x80
#define U_KSLTL      0x40
#define U_ADSR       0x20
#define U_WS         0x10
#define U_FBC        0x08
#define U_FREQ       0x01

#define R_PAN_THRESH  27
#define L_PAN_THRESH  100
#define LEFT_MASK     0xEF
#define RIGHT_MASK    0xDF

#define SEQ_STOPPED  0
#define SEQ_PLAYING  1
#define SEQ_DONE     2

#define PART_VOLUME      7
#define MODULATION       1
#define PANPOT           10
#define EXPRESSION       11
#define SUSTAIN          64
#define PATCH_BANK_SEL   114
#define CHAN_LOCK        110
#define CHAN_PROTECT     111
#define VOICE_PROTECT    112
#define INDIRECT_C_PFX   115
#define FOR_LOOP         116
#define NEXT_LOOP        117
#define CLEAR_BEAT_BAR   118
#define CALLBACK_TRIG    119
#define TIMBRE_PROTECT   113
#define RESET_ALL_CTRLS  121
#define ALL_NOTES_OFF    123

#define DEF_TC_SIZE      3584
#define DEF_SYNTH_VOL    100

const uint8_t CxmiPlayer::dummy_2op[14] = { 0x0E,0x00, 0,0x3F,0xFF,0xFF, 0,0x3F,0xFF,0xFF, 0,0,0,0 };

const uint16_t CxmiPlayer::freq_table[192] = {
    0x02B2, 0x02B4, 0x02B7, 0x02B9, 0x02BC, 0x02BE, 0x02C1, 0x02C3,
    0x02C6, 0x02C9, 0x02CB, 0x02CE, 0x02D0, 0x02D3, 0x02D6, 0x02D8,
    0x02DB, 0x02DD, 0x02E0, 0x02E3, 0x02E5, 0x02E8, 0x02EB, 0x02ED,
    0x02F0, 0x02F3, 0x02F6, 0x02F8, 0x02FB, 0x02FE, 0x0301, 0x0303,
    0x0306, 0x0309, 0x030C, 0x030F, 0x0311, 0x0314, 0x0317, 0x031A,
    0x031D, 0x0320, 0x0323, 0x0326, 0x0329, 0x032B, 0x032E, 0x0331,
    0x0334, 0x0337, 0x033A, 0x033D, 0x0340, 0x0343, 0x0346, 0x0349,
    0x034C, 0x034F, 0x0352, 0x0356, 0x0359, 0x035C, 0x035F, 0x0362,
    0x0365, 0x0368, 0x036B, 0x036F, 0x0372, 0x0375, 0x0378, 0x037B,
    0x037F, 0x0382, 0x0385, 0x0388, 0x038C, 0x038F, 0x0392, 0x0395,
    0x0399, 0x039C, 0x039F, 0x03A3, 0x03A6, 0x03A9, 0x03AD, 0x03B0,
    0x03B4, 0x03B7, 0x03BB, 0x03BE, 0x03C1, 0x03C5, 0x03C8, 0x03CC,
    0x03CF, 0x03D3, 0x03D7, 0x03DA, 0x03DE, 0x03E1, 0x03E5, 0x03E8,
    0x03EC, 0x03F0, 0x03F3, 0x03F7, 0x03FB, 0x03FE,
    0xFE01, 0xFE03, 0xFE05, 0xFE07, 0xFE08, 0xFE0A, 0xFE0C, 0xFE0E,
    0xFE10, 0xFE12, 0xFE14, 0xFE16, 0xFE18, 0xFE1A, 0xFE1C, 0xFE1E,
    0xFE20, 0xFE21, 0xFE23, 0xFE25, 0xFE27, 0xFE29, 0xFE2B, 0xFE2D,
    0xFE2F, 0xFE31, 0xFE34, 0xFE36, 0xFE38, 0xFE3A, 0xFE3C, 0xFE3E,
    0xFE40, 0xFE42, 0xFE44, 0xFE46, 0xFE48, 0xFE4A, 0xFE4C, 0xFE4F,
    0xFE51, 0xFE53, 0xFE55, 0xFE57, 0xFE59, 0xFE5C, 0xFE5E, 0xFE60,
    0xFE62, 0xFE64, 0xFE67, 0xFE69, 0xFE6B, 0xFE6D, 0xFE6F, 0xFE72,
    0xFE74, 0xFE76, 0xFE79, 0xFE7B, 0xFE7D, 0xFE7F, 0xFE82, 0xFE84,
    0xFE86, 0xFE89, 0xFE8B, 0xFE8D, 0xFE90, 0xFE92, 0xFE95, 0xFE97,
    0xFE99, 0xFE9C, 0xFE9E, 0xFEA1, 0xFEA3, 0xFEA5, 0xFEA8, 0xFEAA,
    0xFEAD, 0xFEAF
};

const uint8_t CxmiPlayer::note_octave[128] = {
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,
    2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,
    5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7
};

const uint8_t CxmiPlayer::note_halftone[128] = {
    0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,
    9,10,11,0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,
    6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11
};

const uint8_t CxmiPlayer::note2_octave[96] = {
    0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,
    2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,
    5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7
};

const uint8_t CxmiPlayer::note2_htone[96] = {
    0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,
    9,10,11,0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,
    6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11
};

const uint8_t CxmiPlayer::array0_init[258] = {
    0x20,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
    0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
    0x3F,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,
    0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00
};

const uint8_t CxmiPlayer::array1_init[258] = {
    0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
    0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
    0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
    0x3F,0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,
    0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00
};

const uint8_t CxmiPlayer::vel_graph[16] = {
    82,85,88,91,94,97,100,103,106,109,112,115,118,121,124,127
};

const uint8_t CxmiPlayer::op_0[18] = {
    0,1,2,6,7,8,12,13,14,18,19,20,24,25,26,30,31,32
};

const uint8_t CxmiPlayer::op_1[18] = {
    3,4,5,9,10,11,15,16,17,21,22,23,27,28,29,33,34,35
};

const uint8_t CxmiPlayer::op_index[36] = {
    0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21,
    0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21
};

const uint8_t CxmiPlayer::op_array[36] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

const uint8_t CxmiPlayer::voice_num[18] = {
    0,1,2,3,4,5,6,7,8,0,1,2,3,4,5,6,7,8
};

const uint8_t CxmiPlayer::voice_array[18] = {
    0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1
};

const uint8_t CxmiPlayer::op4_base[18] = {
    1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0
};

const int8_t CxmiPlayer::alt_voice[18] = {
    3,4,5,0,1,2,-1,-1,-1,12,13,14,9,10,11,-1,-1,-1
};

const uint8_t CxmiPlayer::alt_op_0[18] = {
    6,7,8,12,13,14,0xFF,0xFF,0xFF,24,25,26,30,31,32,0xFF,0xFF,0xFF
};

const uint8_t CxmiPlayer::alt_op_1[18] = {
    9,10,11,15,16,17,0xFF,0xFF,0xFF,27,28,29,33,34,35,0xFF,0xFF,0xFF
};

const uint8_t CxmiPlayer::conn_sel[18] = {
    1,2,4,1,2,4,0,0,0,8,16,32,8,16,32,0,0,0
};

const uint8_t CxmiPlayer::op4_voice[6] = {
    0,1,2,9,10,11
};

const uint8_t CxmiPlayer::carrier_01_data[18] = {
    0,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

const uint8_t CxmiPlayer::carrier_23_data[18] = {
    2,2,2,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2
};

/* ======================== CxmiPlayer ======================== */

CPlayer *CxmiPlayer::factory(Copl *newopl) {
    return new CxmiPlayer(newopl);
}

CxmiPlayer::CxmiPlayer(Copl *newopl)
    : CPlayer(newopl)
    , xmi_data(0)
    , xmi_size(0)
    , num_tracks(0)
    , cur_track(-1)
    , title_str(0)
    , author_str(0)
    , title_len(0)
    , author_len(0)
    , ext_bank_data(0)
    , ext_bank_size(0)
    , cache_mem(0)
    , cache_size(0)
    , cache_end(0)
    , nrpn(0)
    , current_chip(-1)
{
    cache_mem = new uint8_t[XMI_CACHE_SIZE];
}

CxmiPlayer::~CxmiPlayer() {
    if (xmi_data)      delete[] xmi_data;
    if (title_str)     delete[] title_str;
    if (author_str)    delete[] author_str;
    if (ext_bank_data) delete[] ext_bank_data;
    delete[] cache_mem;
}

/* ---- utility ---- */

uint32_t CxmiPlayer::read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

uint16_t CxmiPlayer::read_le_16(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

uint32_t CxmiPlayer::read_vln(const uint8_t **ptr) {
    const uint8_t *p = *ptr;
    uint32_t value = 0;
    for (int i = 0; i < 4; i++) {
        value = (value << 7) | (p[i] & 0x7F);
        if ((p[i] & 0x80) == 0) {
            *ptr = p + i + 1;
            return value;
        }
    }
    *ptr = p + 4;
    return value;
}

/* ---- XMI track finding ---- */

uint8_t *CxmiPlayer::find_seq(uint32_t seq_num) {
    uint8_t *p = xmi_data;
    uint32_t form_count = seq_num + 1;
    uint32_t i;

    for (i = 0; i < form_count; ) {
        uint32_t tag = read_be32(p);

        if (tag == 0x43415420U) {           /* "CAT " */
            uint32_t root_len = read_be32(p + 4);
            uint8_t *root_end = p + 8 + root_len;
            p += 12;
            form_count = 0;
            for (;;) {
                if (p >= root_end) return NULL;
                tag = read_be32(p + 8);
                if (tag == 0x584D4944u) {   /* "XMID" */
                    form_count++;
                    if (form_count == seq_num + 1) {
                        if (read_be32(p) != 0x43415420U) return p; /* "CAT " */
                        if (seq_num == 0) return p;
                        return NULL;
                    }
                }
                uint32_t chunk_len = read_be32(p + 4);
                p += 8 + chunk_len;
            }
        }

        if (tag == 0x464F524Du) {           /* "FORM" */
            if (read_be32(p + 8) != 0x584D4944u) { /* "XMID" */
                uint32_t chunk_len = read_be32(p + 4);
                p += 8 + chunk_len;
                continue;
            }
            return p;
        }

        uint32_t chunk_len = read_be32(p + 4);
        p += 8 + chunk_len;
        i++;
    }
    return NULL;
}

/* ---- has_loops ---- */

int CxmiPlayer::has_loops(uint8_t *evnt) {
    if (!evnt) return 0;
    uint32_t len = read_be32(evnt + 4);
    if (len < 3) return 0;
    uint8_t *d = evnt + 8;
    for (uint32_t i = 0; i + 2 < len; i++) {
        if ((d[i] & 0xF0) == 0xB0 && (d[i + 1] == 0x74 || d[i + 1] == 0x75))
            return 1;
    }
    return 0;
}

/* ---- sequence init ---- */

void CxmiPlayer::init_sequence_state() {
    for (uint32_t i = 0; i < XMI_FOR_NEST; i++)
        seq.FOR_loop_cnt[i] = -1;

    for (uint32_t i = 0; i < XMI_NUM_CHANS; i++) {
        seq.chan_map[i] = (uint8_t)i;
        seq.chan_program[i] = 0xFF;
        seq.chan_pitch_l[i] = 0xFF;
        seq.chan_pitch_h[i] = 0xFF;
        seq.chan_indirect[i] = -1;
    }

    for (uint32_t i = 0; i < sizeof(seq.chan_controls); i++)
        ((uint8_t *)&seq.chan_controls)[i] = 0xFF;

    for (uint32_t i = 0; i < XMI_MAX_NOTES; i++)
        seq.note_queue[i].chan = 0xFF;

    seq.interval_cnt = 0;
    seq.note_count = 0;
    seq.vol_percent = DEF_SYNTH_VOL;
    seq.vol_target = DEF_SYNTH_VOL;
    seq.tempo_percent = 100;
    seq.tempo_target = 100;
    seq.tempo_error = 0;
    seq.beat_count = 0;
    seq.measure_count = -1;
    seq.beat_fraction = 0;
    seq.time_fraction = 0;
    seq.time_numerator = 4;
    seq.time_per_beat = 0x07A1200U;
    seq.seq_started = 0;
    seq.status = SEQ_STOPPED;
}

void CxmiPlayer::rewind_seq() {
    uint8_t *form = find_seq((uint32_t)cur_track);
    if (!form) return;

    seq.TIMB = NULL;
    seq.RBRN = NULL;
    seq.EVNT = NULL;

    uint8_t *p = form + 12;
    for (;;) {
        uint32_t tag = read_be32(p);
        uint32_t chunk_len = read_be32(p + 4);

        if (tag == 0x54494D42u) {           /* "TIMB" */
            seq.TIMB = p;
        } else if (tag == 0x5242524Eu) {    /* "RBRN" */
            seq.RBRN = p;
        } else if (tag == 0x45564E54u) {    /* "EVNT" */
            seq.EVNT = p;
            break;
        }

        p += 8 + chunk_len;
    }

    init_sequence_state();
    seq.status = SEQ_PLAYING;
    seq.seq_started = 1;
    seq.EVNT_ptr = seq.EVNT + 8;
}

/* ---- OPL register writing ---- */

void CxmiPlayer::opl_write_chip(uint8_t chip, uint8_t reg, uint8_t val) {
    if ((int)chip != current_chip) {
        opl->setchip(chip);
        current_chip = (int)chip;
    }
    opl->write(reg, val);
}

void CxmiPlayer::write_reg(uint8_t reg, uint8_t bank, uint8_t val) {
    opl_write_chip(bank, reg, val);
}

void CxmiPlayer::update_reg(uint8_t oper, uint8_t base, uint8_t val) {
    uint8_t chip = op_array[oper];
    uint8_t reg = (uint8_t)((op_index[oper] + base) & 0xFF);
    opl_write_chip(chip, reg, val);
}

void CxmiPlayer::send_byte(uint8_t voice, uint8_t base, uint8_t val) {
    uint8_t chip = voice_array[voice];
    uint8_t reg = (uint8_t)((voice_num[voice] + base) & 0xFF);
    opl_write_chip(chip, reg, val);
}

/* ---- synth reset/init ---- */

void CxmiPlayer::reset_synth() {
    write_reg(0x05, 1, 0x01);
    write_reg(0x04, 1, 0x00);
    conn_shadow = 0;

    for (int bx = 1; bx <= 0xF5; bx++)
        write_reg((uint8_t)bx, 0, array0_init[bx - 1]);

    for (int bx = 1; bx <= 0xF5; bx++)
        write_reg((uint8_t)bx, 1, array1_init[bx - 1]);
}

void CxmiPlayer::init_synth() {
    note_event_ctr = 0;
    for (int i = 0; i < XMI_MAX_TIMBS; i++) timb_attribs[i] = 0;
    for (int i = 0; i < XMI_NUM_CHANS; i++) {
        MIDI_timbre[i] = -1;
        MIDI_voices[i] = 0;
        MIDI_program[i] = -1;
        MIDI_bank[i] = 0;
        MIDI_vol[i] = 100;
        MIDI_express[i] = 127;
        MIDI_pan[i] = 64;
        MIDI_sus[i] = 0;
        MIDI_pitch_l[i] = 0;
        MIDI_pitch_h[i] = 64;
    }
    for (int i = 0; i < XMI_NUM_SLOTS; i++) S_status[i] = XMI_SLOT_FREE;
    for (int i = 0; i < XMI_NUM_VOICES; i++) V_channel[i] = -1;
    for (int i = 0; i < 128; i++) RBS_timbres[i] = -1;
    rover_2op = -1;
    rover_4op = -1;
    cache_end = 0;
}

void CxmiPlayer::reset_yamaha() {
    init_synth();
    reset_synth();
}

/* ---- timbre cache ---- */

void CxmiPlayer::yamaha_define_cache() {
    cache_size = XMI_CACHE_SIZE;
    cache_end = 0;
}

uint32_t CxmiPlayer::index_timbre(uint16_t gnum) {
    uint8_t num = (uint8_t)(gnum & 0xFF);
    uint8_t bank = (uint8_t)((gnum >> 8) & 0xFF);
    for (int i = 0; i < XMI_MAX_TIMBS; i++) {
        if (!(timb_attribs[i] & 0x80)) continue;
        if (timb_bank[i] != bank) continue;
        if (timb_num[i] == num) return (uint32_t)i;
    }
    return 0xFFFFFFFF;
}

void CxmiPlayer::delete_LRU() {
    int lru_idx = -1;
    uint32_t lru_ctr = 0xFFFFFFFF;
    for (int i = 0; i < XMI_MAX_TIMBS; i++) {
        if (!(timb_attribs[i] & 0x80)) continue;
        if (timb_attribs[i] & 0x40) continue;
        if (timb_hist[i] >= lru_ctr) continue;
        lru_ctr = timb_hist[i];
        lru_idx = i;
    }
    if (lru_idx < 0) return;

    uint32_t toff = timb_offsets[lru_idx];
    uint8_t *timb_ptr = cache_mem + toff;
    uint16_t tsize = read_le_16(timb_ptr);

    uint8_t *dst = cache_mem + toff;
    uint8_t *src = dst + tsize;
    uint32_t count = (uint32_t)((cache_mem + cache_end) - src);
    for (int j = 0; j < (int)count; j++) dst[j] = src[j];

    timb_attribs[lru_idx] = 0;
    cache_end -= tsize;

    for (int ch = 0; ch < XMI_NUM_CHANS; ch++) {
        if (MIDI_timbre[ch] == lru_idx) MIDI_timbre[ch] = -1;
    }
    for (int ch = 0; ch < 128; ch++) {
        if (RBS_timbres[ch] == lru_idx) RBS_timbres[ch] = -1;
    }
    for (int ch = 0; ch < XMI_MAX_TIMBS; ch++) {
        if (!(timb_attribs[ch] & 0x80)) continue;
        if (timb_offsets[ch] <= toff) continue;
        timb_offsets[ch] -= tsize;
    }
    for (int ch = 0; ch < XMI_NUM_SLOTS; ch++) {
        if (S_status[ch] == XMI_SLOT_FREE) continue;
        uint32_t off = ((uint32_t)S_timbre_off_h[ch] << 8) | S_timbre_off_l[ch];
        if (off <= toff) continue;
        if (off == toff) {
            release_voice(ch);
            S_status[ch] = XMI_SLOT_FREE;
        } else {
            off -= tsize;
            S_timbre_off_l[ch] = (uint8_t)(off & 0xFF);
            S_timbre_off_h[ch] = (uint8_t)((off >> 8) & 0xFF);
        }
    }
}

void CxmiPlayer::do_install_timbre(uint16_t gnum, const uint8_t *data) {
    uint8_t num = (uint8_t)(gnum & 0xFF);
    uint8_t bank = (uint8_t)((gnum >> 8) & 0xFF);

    uint32_t idx = index_timbre(gnum);
    if (idx != 0xFFFFFFFF) {
        if (data != NULL) {
            for (int ch = 0; ch < XMI_NUM_CHANS; ch++) {
                if (MIDI_program[ch] == (int8_t)num && MIDI_bank[ch] == bank)
                    MIDI_timbre[ch] = (int8_t)idx;
            }
        }
        return;
    }

    if (data == NULL) return;

    uint16_t tsize = read_le_16(data);

    while ((cache_end + tsize) > cache_size) delete_LRU();

    uint8_t slot = 0xFF;
    for (int i = 0; i < XMI_MAX_TIMBS; i++) {
        if (!(timb_attribs[i] & 0x80)) { slot = (uint8_t)i; break; }
    }
    if (slot == 0xFF) { delete_LRU(); slot = 0; }

    uint32_t off = cache_end;
    uint8_t *dst = cache_mem + off;
    for (uint32_t k = 0; k < tsize; k++) dst[k] = data[k];

    timb_num[slot] = num;
    timb_bank[slot] = bank;
    timb_attribs[slot] = 0x80;
    note_event_ctr++;
    timb_hist[slot] = note_event_ctr;
    timb_offsets[slot] = off;
    cache_end += tsize;

    for (int ch = 0; ch < XMI_NUM_CHANS; ch++) {
        if (MIDI_program[ch] == (int8_t)num && MIDI_bank[ch] == bank)
            MIDI_timbre[ch] = (int8_t)slot;
    }
}

void CxmiPlayer::yamaha_install_timbre(uint32_t bank, uint32_t patch, const uint8_t *data) {
    uint16_t gnum = (uint16_t)((bank << 8) | patch);
    do_install_timbre(gnum, data);
}

void CxmiPlayer::yamaha_protect_timbre(uint32_t bank, uint32_t patch) {
    uint16_t gnum = (uint16_t)((bank << 8) | patch);
    uint32_t idx = index_timbre(gnum);
    if (idx == 0xFFFFFFFF) return;
    timb_attribs[idx] |= 0x40;
}

void CxmiPlayer::yamaha_unprotect_timbre(uint32_t bank, uint32_t patch) {
    uint16_t gnum = (uint16_t)((bank << 8) | patch);
    uint32_t idx = index_timbre(gnum);
    if (idx == 0xFFFFFFFF) return;
    timb_attribs[idx] &= 0xBF;
}

int32_t CxmiPlayer::yamaha_timbre_status(uint32_t bank, uint32_t patch) {
    uint16_t gnum = (uint16_t)((bank << 8) | patch);
    uint32_t idx = index_timbre(gnum);
    if (idx == 0xFFFFFFFF) return 0;
    return (int32_t)(timb_offsets[idx] + 1);
}

uint32_t CxmiPlayer::timbre_request() {
    const uint8_t *timb = seq.TIMB;
    if (!timb) return 0xFFFFFFFFU;

    if (timb[0] != 'T' || timb[1] != 'I' || timb[2] != 'M' || timb[3] != 'B')
        return 0xFFFFFFFFU;

    uint32_t chunk_len = read_be32(timb + 4);
    if (chunk_len < 2) return 0xFFFFFFFFU;

    const uint8_t *data = timb + 8;
    uint32_t count = read_le_16(data);
    if (count == 0) return 0xFFFFFFFFU;

    uint32_t offset = 2;
    for (uint32_t i = 0; i < count; i++) {
        if (offset + 1 >= chunk_len) break;
        uint32_t patch = (uint32_t)data[offset];
        uint32_t bank  = (uint32_t)data[offset + 1];
        if (yamaha_timbre_status(bank, patch) == 0) {
            return (bank << 8) | patch;
        }
        offset += 2;
    }
    return 0xFFFFFFFFU;
}

int CxmiPlayer::install_sequence_timbres() {
    /* prefer an external GTL/OPL bank loaded alongside the XMI file;
     * fall back to the built-in FAT bank when none was found */
    const unsigned char *fat = ext_bank_data ? ext_bank_data : g_fat_opl_data;
    unsigned int        fat_len = ext_bank_data ? ext_bank_size : g_fat_opl_len;

    uint32_t treq;
    int count = 0;
    while ((treq = timbre_request()) != 0xFFFFFFFFU) {
        uint8_t bank = (uint8_t)(treq >> 8);
        uint8_t patch = (uint8_t)(treq & 0xFF);
        const unsigned char *timb = NULL;

        unsigned int pos = 0;
        while (pos + 6 <= fat_len) {
            unsigned char p = fat[pos];
            unsigned char b = fat[pos + 1];
            unsigned int offset = (uint32_t)fat[pos+2] | ((uint32_t)fat[pos+3] << 8) |
                                  ((uint32_t)fat[pos+4] << 16) | ((uint32_t)fat[pos+5] << 24);
            if (b == 0xFF) break;
            if (b == bank && p == patch) {
                if (offset > 0 && offset + 2 <= fat_len)
                    timb = fat + offset;
                break;
            }
            pos += 6;
        }

        if (timb) {
            yamaha_install_timbre(bank, patch, timb);
            count++;
        } else {
            yamaha_install_timbre(bank, patch, dummy_2op);
        }
    }
    return count;
}

/* ---- voice management ---- */

void CxmiPlayer::release_voice(int32_t slot) {
    if (S_voice[slot] < 0) return;

    S_BLOCK[slot] &= 0xDF;
    S_update[slot] |= U_FREQ;
    update_voice(slot);

    int32_t ch = S_channel[slot];
    MIDI_voices[ch]--;
    int32_t v = S_voice[slot];

    if (S_type[slot] == XMI_OPL3_INST) {
        V_channel[v + 3] = -1;
    }
    V_channel[v] = -1;
    S_voice[slot] = -1;

    if (S_type[slot] == XMI_OPL3_INST || S_type[slot] == XMI_BNK_INST) {
        S_status[slot] = XMI_SLOT_FREE;
    }
}

void CxmiPlayer::assign_voice(int32_t slot) {
    if (S_type[slot] == XMI_OPL3_INST) {
        int dx = rover_4op;
        for (int v = 0; v < XMI_NUM_4OP; v++) {
            dx++;
            if (dx >= XMI_NUM_4OP) dx = 0;
            rover_4op = dx;
            int vi = op4_voice[dx];
            if (V_channel[vi] >= 0) continue;
            if (V_channel[vi + 3] >= 0) continue;
            S_voice[slot] = (int8_t)vi;
            int ch = S_channel[slot];
            MIDI_voices[ch]++;
            V_channel[vi] = (int8_t)ch;
            V_channel[vi + 3] = (int8_t)ch;
            S_update[slot] = U_ALL_REGS;
            update_voice(slot);
            return;
        }
        update_priority();
        return;
    }

    int dx = rover_2op;
    for (int v = 0; v < XMI_NUM_VOICES; v++) {
        dx++;
        if (dx >= XMI_NUM_VOICES) dx = 0;
        rover_2op = dx;
        if (V_channel[dx] >= 0) continue;
        S_voice[slot] = (int8_t)dx;
        int ch = S_channel[slot];
        MIDI_voices[ch]++;
        V_channel[dx] = (int8_t)ch;
        S_update[slot] = U_ALL_REGS;
        update_voice(slot);
        return;
    }
    update_priority();
}

void CxmiPlayer::BNK_phase(int32_t slot) {
    uint32_t off = ((uint32_t)S_timbre_off_h[slot] << 8) | S_timbre_off_l[slot];
    uint8_t *tptr = cache_mem + off;

    S_BLOCK[slot] = 0x20;
    S_type[slot] = XMI_BNK_INST;
    S_duration[slot] = 0xFFFF;
    S_p_val[slot] = 32767;

    uint8_t fb_c = tptr[8];
    uint8_t m_ksltl = tptr[4];
    uint8_t c_ksltl = tptr[10];
    uint8_t m_avekm = tptr[3];
    uint8_t c_avekm = tptr[9];

    S_FBC[slot] = fb_c & 1;
    S_fb_val[slot] = ((uint16_t)(fb_c & 0x0E)) << 3;

    S_KSLTL_0[slot] = m_ksltl & 0xC0;
    S_v0_val[slot] = (uint16_t)(((~m_ksltl) & 0x3F) << 2);

    S_KSLTL_1[slot] = c_ksltl & 0xC0;
    S_v1_val[slot] = (uint16_t)(((~c_ksltl) & 0x3F) << 2);

    S_AVEKM_0[slot] = m_avekm;
    S_m0_val[slot] = ((uint16_t)m_avekm & 0x0F) << 4;

    S_AVEKM_1[slot] = c_avekm;
    S_m1_val[slot] = ((uint16_t)c_avekm & 0x0F) << 4;

    S_AD_0[slot] = tptr[5];
    S_SR_0[slot] = tptr[6];
    S_AD_1[slot] = tptr[11];
    S_SR_1[slot] = tptr[12];

    S_ws_val[slot] = ((uint16_t)tptr[13] << 8) | tptr[7];

    S_scale_01[slot] = (uint8_t)(S_FBC[slot] | 2);
    S_update[slot] = U_ALL_REGS;
}

void CxmiPlayer::OPL_phase(int32_t slot) {
    BNK_phase(slot);

    uint32_t off = ((uint32_t)S_timbre_off_h[slot] << 8) | S_timbre_off_l[slot];
    uint8_t *tptr = cache_mem + off;

    S_type[slot] = XMI_OPL3_INST;

    uint8_t fb_c = tptr[8];
    S_FBC[slot] = (S_FBC[slot] & 1) | ((fb_c & 0x80) >> 6);

    uint8_t c01 = carrier_01_data[S_FBC[slot] & 3];
    uint8_t c23 = carrier_23_data[S_FBC[slot] & 3];
    S_scale_01[slot] = c01;
    S_scale_23[slot] = c23;

    uint8_t om_ksltl = tptr[15];
    S_KSLTL_2[slot] = om_ksltl & 0xC0;
    S_v2_val[slot] = (uint16_t)(((~om_ksltl) & 0x3F) << 2);

    uint8_t oc_ksltl = tptr[21];
    S_KSLTL_3[slot] = oc_ksltl & 0xC0;
    S_v3_val[slot] = (uint16_t)(((~oc_ksltl) & 0x3F) << 2);

    uint8_t om_avekm = tptr[14];
    S_AVEKM_2[slot] = om_avekm & 0xF0;
    S_m2_val[slot] = ((uint16_t)om_avekm & 0x0F) << 4;

    uint8_t oc_avekm = tptr[20];
    S_AVEKM_3[slot] = oc_avekm & 0xF0;
    S_m3_val[slot] = ((uint16_t)oc_avekm & 0x0F) << 4;

    S_AD_2[slot] = tptr[16];
    S_SR_2[slot] = tptr[17];
    S_AD_3[slot] = tptr[22];
    S_SR_3[slot] = tptr[23];

    S_ws_val_2[slot] = ((uint16_t)tptr[24] << 8) | tptr[18];
}

void CxmiPlayer::update_voice(int32_t slot) {
    if (S_voice[slot] < 0) return;
    uint8_t vol = 0;
    if (S_update[slot] & U_KSLTL) {
        int32_t ch = S_channel[slot] & 0x0F;
        uint32_t t = (uint32_t)MIDI_vol[ch] * (uint32_t)MIDI_express[ch];
        t = (t * 2) >> 8;
        if (t) t++;
        uint32_t v = t * S_velocity[slot];
        v = (v * 2) >> 8;
        if (v) v++;
        vol = (uint8_t)v;
    }

    int arr = (S_type[slot] == XMI_OPL3_INST) ? 1 : 0;

    int32_t vi = S_voice[slot];
    uint8_t conn = conn_sel[vi];
    uint8_t cur_shadow = conn_shadow;
    if (arr) {
        uint8_t ncs = (uint8_t)(cur_shadow | conn);
        if (ncs != cur_shadow) {
            conn_shadow = ncs;
            write_reg(0x04, 1, ncs);
        }
    } else {
        uint8_t ncs = (uint8_t)(cur_shadow & ~conn);
        if (ncs != cur_shadow) {
            conn_shadow = ncs;
            write_reg(0x04, 1, ncs);
        }
    }

    do {
        int32_t vi = S_voice[slot];
        uint8_t op_a = arr ? alt_op_0[vi] : op_0[vi];
        uint8_t op_b = arr ? alt_op_1[vi] : op_1[vi];

        if (S_update[slot] & U_AVEKM) {
            int32_t ch = S_channel[slot] & 0x0F;
            uint8_t am_vib = (MIDI_mod[ch] >= 64) ? 0x40 : 0x00;

            if (arr) {
                uint8_t r2 = (uint8_t)((((S_m2_val[slot] >> 4) & 0x0F) | am_vib | S_AVEKM_2[slot]));
                update_reg(op_a, 0x20, r2);
                uint8_t r3 = (uint8_t)((((S_m3_val[slot] >> 4) & 0x0F) | am_vib | S_AVEKM_3[slot]));
                update_reg(op_b, 0x20, r3);
            } else {
                uint8_t r0 = (uint8_t)((((S_m0_val[slot] >> 4) & 0x0F) | am_vib | S_AVEKM_0[slot]));
                update_reg(op_a, 0x20, r0);
                uint8_t r1 = (uint8_t)((((S_m1_val[slot] >> 4) & 0x0F) | am_vib | S_AVEKM_1[slot]));
                update_reg(op_b, 0x20, r1);
            }
        }

        if (S_update[slot] & U_KSLTL) {
            if (arr) {
                uint8_t v2_lev = (uint8_t)((S_v2_val[slot] >> 2) & 0x3F);
                if (S_scale_23[slot] & 1)
                    v2_lev = (uint8_t)((((uint32_t)v2_lev * vol)) / 127);
                update_reg(op_a, 0x40, (uint8_t)((~v2_lev & 0x3F) | S_KSLTL_2[slot]));

                uint8_t v3_lev = (uint8_t)((S_v3_val[slot] >> 2) & 0x3F);
                if (S_scale_23[slot] & 2)
                    v3_lev = (uint8_t)((((uint32_t)v3_lev * vol)) / 127);
                update_reg(op_b, 0x40, (uint8_t)((~v3_lev & 0x3F) | S_KSLTL_3[slot]));
            } else {
                uint8_t v0_lev = (uint8_t)((S_v0_val[slot] >> 2) & 0x3F);
                if (S_scale_01[slot] & 1)
                    v0_lev = (uint8_t)((((uint32_t)v0_lev * vol)) / 127);
                update_reg(op_a, 0x40, (uint8_t)((~v0_lev & 0x3F) | S_KSLTL_0[slot]));

                uint8_t v1_lev = (uint8_t)((S_v1_val[slot] >> 2) & 0x3F);
                if (S_scale_01[slot] & 2)
                    v1_lev = (uint8_t)((((uint32_t)v1_lev * vol)) / 127);
                update_reg(op_b, 0x40, (uint8_t)((~v1_lev & 0x3F) | S_KSLTL_1[slot]));
            }
        }

        if (S_update[slot] & U_ADSR) {
            if (arr) {
                update_reg(op_a, 0x60, S_AD_2[slot]);
                update_reg(op_b, 0x60, S_AD_3[slot]);
                update_reg(op_a, 0x80, S_SR_2[slot]);
                update_reg(op_b, 0x80, S_SR_3[slot]);
            } else {
                update_reg(op_a, 0x60, S_AD_0[slot]);
                update_reg(op_b, 0x60, S_AD_1[slot]);
                update_reg(op_a, 0x80, S_SR_0[slot]);
                update_reg(op_b, 0x80, S_SR_1[slot]);
            }
        }

        if (S_update[slot] & U_WS) {
            if (arr) {
                update_reg(op_a, 0xE0, (uint8_t)(S_ws_val_2[slot] & 0xFF));
                update_reg(op_b, 0xE0, (uint8_t)(S_ws_val_2[slot] >> 8));
            } else {
                update_reg(op_a, 0xE0, (uint8_t)(S_ws_val[slot] & 0xFF));
                update_reg(op_b, 0xE0, (uint8_t)(S_ws_val[slot] >> 8));
            }
        }

        if (S_update[slot] & U_FBC) {
            int fbc = (uint8_t)(((S_FBC[slot] & 1) | 0x30 | ((S_fb_val[slot] >> 3) & 0x0E)) & 0xFF);
            uint8_t pan = MIDI_pan[S_channel[slot] & 0x0F];
            if (pan <= R_PAN_THRESH) fbc &= RIGHT_MASK;
            else if (pan >= L_PAN_THRESH) fbc &= LEFT_MASK;
            send_byte((uint8_t)(arr ? (vi + 3) : vi), 0xC0, (uint8_t)fbc);
        }

        if (S_update[slot] & U_FREQ) {
            if (arr) {
                if (!(S_BLOCK[slot] & 0x20)) {
                    uint8_t kb = (uint8_t)(S_KBF_shadow[slot] & 0xDF);
                    send_byte((uint8_t)(vi + 3), 0xB0, kb);
                } else {
                    int32_t ch = S_channel[slot] & 0x0F;
                    int32_t pb = (((int32_t)MIDI_pitch_h[ch] << 7) | (int32_t)MIDI_pitch_l[ch]) - 0x2000;
                    pb >>= 5;
                    pb *= DEF_PITCH_RANGE;
                    int32_t note = S_note[slot] + S_transpose[slot];
                    while (note < 0) note += 12;
                    while (note >= 96) note -= 12;
                    int32_t val = note * 256 + pb + 8;
                    val >>= 4;
                    val -= 192;
                    while (val < 0) val += 192;
                    while (val >= 1536) val -= 192;
                    int idx = val >> 4;
                    uint8_t htone = note2_htone[idx];
                    int word_idx = (htone << 4) + (val & 0x0F);
                    uint16_t fval = freq_table[word_idx];
                    int oct = note2_octave[idx];
                    oct--;
                    if (fval >= 0x8000) oct++;
                    if (oct < 0) { oct++; fval >>= 1; }
                    uint8_t blk_fnum = (uint8_t)((((uint8_t)oct << 2) & 0x1C) | ((fval >> 8) & 3));
                    uint8_t fnum_lo = (uint8_t)(fval & 0xFF);
                    S_KBF_shadow[slot] = (uint8_t)((blk_fnum & 0xDF) | (S_BLOCK[slot] & 0x20));
                    send_byte((uint8_t)(vi + 3), 0xA0, fnum_lo);
                    send_byte((uint8_t)(vi + 3), 0xB0, S_KBF_shadow[slot]);
                }
            } else {
                if (!(S_BLOCK[slot] & 0x20)) {
                    uint8_t kb = (uint8_t)(S_KBF_shadow[slot] & 0xDF);
                    send_byte((uint8_t)vi, 0xB0, kb);
                } else {
                    int32_t ch = S_channel[slot] & 0x0F;
                    int32_t pb = (((int32_t)MIDI_pitch_h[ch] << 7) | (int32_t)MIDI_pitch_l[ch]) - 0x2000;
                    pb >>= 5;
                    pb *= DEF_PITCH_RANGE;
                    int32_t note = S_note[slot] + S_transpose[slot];
                    while (note < 0) note += 12;
                    while (note >= 96) note -= 12;
                    int32_t val = note * 256 + pb + 8;
                    val >>= 4;
                    val -= 192;
                    while (val < 0) val += 192;
                    while (val >= 1536) val -= 192;
                    int idx = val >> 4;
                    uint8_t htone = note2_htone[idx];
                    int word_idx = (htone << 4) + (val & 0x0F);
                    uint16_t fval = freq_table[word_idx];
                    int oct = note2_octave[idx];
                    oct--;
                    if (fval >= 0x8000) oct++;
                    if (oct < 0) { oct++; fval >>= 1; }
                    uint8_t blk_fnum = (uint8_t)((((uint8_t)oct << 2) & 0x1C) | ((fval >> 8) & 3));
                    uint8_t fnum_lo = (uint8_t)(fval & 0xFF);
                    S_KBF_shadow[slot] = (uint8_t)((blk_fnum & 0xDF) | (S_BLOCK[slot] & 0x20));
                    send_byte((uint8_t)vi, 0xA0, fnum_lo);
                    send_byte((uint8_t)vi, 0xB0, S_KBF_shadow[slot]);
                }
            }
        }
    } while (arr-- > 0);

    S_update[slot] &= (uint8_t)(~(U_AVEKM | U_KSLTL | U_ADSR | U_WS | U_FBC | U_FREQ));
}

void CxmiPlayer::update_priority() {
    int slot_cnt = 0;
    int high_unvoiced = -1;
    int low_voiced = -1;
    uint32_t low_voiced_pri = 0xFFFF;
    int low_4op = -1;
    uint32_t low_4op_pri = 0xFFFF;

    for (int si = 0; si < XMI_NUM_SLOTS; si++) {
        if (S_status[si] == XMI_SLOT_FREE) continue;
        slot_cnt++;
        int32_t ch = S_channel[si] & 0x0F;
        uint32_t pri = (MIDI_vprot[ch] >= 64) ? 0xFFFF : S_p_val[si];
        pri -= MIDI_voices[ch];
        if ((int32_t)pri < 0) pri = 0;
        S_V_priority[si] = (uint16_t)pri;
    }

    for (int si = 0; si < XMI_NUM_SLOTS; si++) {
        if (S_status[si] == XMI_SLOT_FREE) continue;
        uint32_t pri = S_V_priority[si];
        int vi = S_voice[si];

        if (vi < 0) {
            if (pri > (uint32_t)high_unvoiced) high_unvoiced = si;
        } else {
            if (op4_base[vi] && pri < low_4op_pri) {
                low_4op_pri = pri;
                low_4op = si;
            }
            if (pri < low_voiced_pri) {
                low_voiced_pri = pri;
                low_voiced = si;
            }
        }
    }

    if (high_unvoiced < 0 || low_voiced < 0) return;

    int victim = low_voiced;
    int new_slot = high_unvoiced;
    if (S_type[new_slot] == XMI_OPL3_INST) {
        victim = low_4op;
        if (S_type[victim] == XMI_OPL3_INST) {
            int8_t alt_v = alt_voice[S_voice[victim]];
            if (alt_v >= 0) {
                for (int k = 0; k < XMI_NUM_SLOTS; k++) {
                    if (S_status[k] == XMI_SLOT_FREE) continue;
                    if (S_voice[k] != alt_v) continue;
                    release_voice(k);
                    break;
                }
            }
        }
    }

    int old_v = S_voice[victim];
    release_voice(victim);

    S_voice[new_slot] = (int8_t)old_v;
    int ch = S_channel[new_slot] & 0x0F;
    MIDI_voices[ch]++;
    V_channel[old_v] = (int8_t)ch;
    if (S_type[new_slot] == XMI_OPL3_INST)
        V_channel[old_v + 3] = (int8_t)ch;
    S_update[new_slot] = U_ALL_REGS;
    update_voice(new_slot);
}

/* ---- yamaha MIDI interface ---- */

void CxmiPlayer::yamaha_note_on(uint32_t chan, uint32_t note, uint32_t vel) {
    if (chan >= XMI_NUM_CHANS) return;

    int timb_idx;
    if (chan == 9) {
        timb_idx = RBS_timbres[note];
        if (timb_idx < 0) {
            uint16_t gnum = (0x7F << 8) | (note & 0x7F);
            timb_idx = (int)index_timbre(gnum);
            RBS_timbres[note] = (int8_t)timb_idx;
        }
        if (timb_idx < 0) return;
    } else {
        if (MIDI_timbre[chan] < 0) return;
        timb_idx = MIDI_timbre[chan];
    }

    uint32_t off = timb_offsets[timb_idx];
    uint8_t *tptr = cache_mem + off;

    note_event_ctr++;
    timb_hist[timb_idx] = note_event_ctr;

    int slot = -1;
    for (int si = 0; si < XMI_NUM_SLOTS; si++) {
        if (S_status[si] == XMI_SLOT_FREE) { slot = si; break; }
    }
    if (slot < 0) return;

    S_channel[slot] = (uint8_t)chan;
    S_keynum[slot] = (uint8_t)note;

    uint8_t transp = tptr[2];
    if (chan != 9) {
        S_transpose[slot] = (int8_t)transp;
        S_note[slot] = (uint8_t)note;
    } else {
        S_transpose[slot] = 0;
        S_note[slot] = (uint8_t)(note + transp);
    }

    uint8_t v = (uint8_t)(vel >> 3);
    if (v < 16) v = vel_graph[v];
    S_velocity[slot] = v;

    uint32_t tsize = read_le_16(tptr);
    S_timbre_off_l[slot] = (uint8_t)(off & 0xFF);
    S_timbre_off_h[slot] = (uint8_t)((off >> 8) & 0xFF);

    S_status[slot] = XMI_SLOT_KEYON;
    S_sustain[slot] = 0;

    if (tsize == 25) OPL_phase(slot);
    else BNK_phase(slot);

    S_voice[slot] = -1;
    assign_voice(slot);
}

void CxmiPlayer::yamaha_note_off(uint32_t chan, uint32_t note) {
    if (chan >= XMI_NUM_CHANS) return;
    for (int si = 0; si < XMI_NUM_SLOTS; si++) {
        if (S_status[si] != XMI_SLOT_KEYON) continue;
        if (S_keynum[si] != note) continue;
        if (S_channel[si] != chan) continue;

        if (MIDI_sus[chan] < 64) {
            if (S_type[si] == XMI_OPL3_INST || S_type[si] == XMI_BNK_INST) {
                release_voice(si);
                S_status[si] = XMI_SLOT_FREE;
            } else {
                S_duration[si] = 1;
            }
        } else {
            S_sustain[si] = 1;
        }
    }
}

void CxmiPlayer::yamaha_controller(uint32_t chan, uint32_t ctrl, uint32_t val) {
    if (chan >= XMI_NUM_CHANS) return;

    switch (ctrl) {
        case 0:    MIDI_bank[chan] = (uint8_t)val; break;
        case 1:    MIDI_mod[chan] = (uint8_t)val; break;
        case 7:    MIDI_vol[chan] = (uint8_t)val; break;
        case 10:   MIDI_pan[chan] = (uint8_t)val; break;
        case 11:   MIDI_express[chan] = (uint8_t)val; break;
        case 64:
            MIDI_sus[chan] = (uint8_t)val;
            if (val < 64) {
                for (int si = 0; si < XMI_NUM_SLOTS; si++) {
                    if (S_status[si] == XMI_SLOT_FREE) continue;
                    if (S_channel[si] != chan) continue;
                    if (!S_sustain[si]) continue;
                    S_sustain[si] = 0;
                    yamaha_note_off(chan, S_keynum[si]);
                }
            }
            break;
        case 96: case 97: break;
        case VOICE_PROTECT: MIDI_vprot[chan] = (uint8_t)val; break;
        case TIMBRE_PROTECT:
            if (MIDI_timbre[chan] >= 0) {
                if (val >= 64) timb_attribs[MIDI_timbre[chan]] |= 0x40;
                else           timb_attribs[MIDI_timbre[chan]] &= 0xBF;
            }
            break;
        case RESET_ALL_CTRLS:
            MIDI_mod[chan] = 0;
            MIDI_express[chan] = 127;
            MIDI_sus[chan] = 0;
            MIDI_pitch_l[chan] = 0;
            MIDI_pitch_h[chan] = 64;
            for (int si = 0; si < XMI_NUM_SLOTS; si++) {
                if (S_status[si] == XMI_SLOT_FREE) continue;
                if (S_channel[si] != chan) continue;
                if (S_sustain[si]) { S_sustain[si] = 0; yamaha_note_off(chan, S_keynum[si]); }
            }
            for (int si = 0; si < XMI_NUM_SLOTS; si++) {
                if (S_status[si] == XMI_SLOT_FREE) continue;
                if (S_channel[si] != chan) continue;
                S_update[si] |= U_ALL_REGS;
                update_voice(si);
            }
            break;
        case ALL_NOTES_OFF:
            for (int si = 0; si < XMI_NUM_SLOTS; si++) {
                if (S_status[si] != XMI_SLOT_KEYON) continue;
                if (S_channel[si] != chan) continue;
                yamaha_note_off(chan, S_keynum[si]);
            }
            break;
    }

    if (ctrl == 1 || ctrl == 7 || ctrl == 11 || ctrl == 10) {
        uint8_t upd_flag = (ctrl == 1) ? U_AVEKM : (ctrl == 10) ? U_FBC : U_KSLTL;
        for (int si = 0; si < XMI_NUM_SLOTS; si++) {
            if (S_status[si] == XMI_SLOT_FREE) continue;
            if (S_channel[si] != chan) continue;
            S_update[si] |= upd_flag;
            update_voice(si);
        }
    }
}

void CxmiPlayer::yamaha_program_change(uint32_t chan, uint32_t program) {
    if (chan >= XMI_NUM_CHANS) return;
    MIDI_program[chan] = (int8_t)program;
    uint16_t gnum = (uint16_t)((MIDI_bank[chan] << 8) | (program & 0xFF));
    uint32_t idx = index_timbre(gnum);
    MIDI_timbre[chan] = (int8_t)idx;
}

void CxmiPlayer::yamaha_pitch_bend(uint32_t chan, uint32_t pitch_l, uint32_t pitch_h) {
    if (chan >= XMI_NUM_CHANS) return;
    MIDI_pitch_l[chan] = (uint8_t)pitch_l;
    MIDI_pitch_h[chan] = (uint8_t)pitch_h;
    for (int si = 0; si < XMI_NUM_SLOTS; si++) {
        if (S_status[si] == XMI_SLOT_FREE) continue;
        if (S_channel[si] != chan) continue;
        S_update[si] |= U_FREQ;
        update_voice(si);
    }
}

void CxmiPlayer::send_MIDI_message(uint32_t status, uint32_t d1, uint32_t d2) {
    uint32_t chan = status & 0x0F;
    uint32_t type = status & 0xF0;

    if (type == 0x80 || (type == 0x90 && d2 == 0)) {
        yamaha_note_off(chan, d1);
        return;
    }
    if (type == 0x90) {
        yamaha_note_on(chan, d1, d2);
        return;
    }
    if (type == 0xB0) {
        yamaha_controller(chan, d1, d2);
        return;
    }
    if (type == 0xC0) {
        yamaha_program_change(chan, d1);
        return;
    }
    if (type == 0xE0) {
        yamaha_pitch_bend(chan, d1, d2);
        return;
    }
}

/* ---- note queue flush ---- */

void CxmiPlayer::flush_note_queue() {
    for (uint32_t i = 0; i < XMI_MAX_NOTES; i++) {
        if (seq.note_queue[i].chan == 0xFF) continue;
        uint32_t note = seq.note_queue[i].num;
        uint32_t phys = seq.chan_map[seq.note_queue[i].chan & 0x0F];
        seq.note_queue[i].chan = 0xFF;
        if (phys < XMI_NUM_CHANS)
            yamaha_note_off(phys, note);
    }
    seq.note_count = 0;
}

void CxmiPlayer::flush_channel_notes(uint32_t chan) {
    for (uint32_t j = 0; j < XMI_MAX_NOTES; j++) {
        if (seq.note_queue[j].chan == 0xFF) continue;
        if ((seq.note_queue[j].chan & 0x0F) != (uint8_t)chan) continue;
        uint32_t note = seq.note_queue[j].num;
        uint32_t phys = seq.chan_map[chan];
        seq.note_queue[j].chan = 0xFF;
        if (phys < XMI_NUM_CHANS)
            yamaha_note_off(phys, note);
    }
    seq.note_count = 0;
}

/* ---- XMIDI event handlers ---- */

uint32_t CxmiPlayer::xmidi32_XMIDI_control(uint32_t log_chan, uint32_t ctrl, uint32_t val) {
    if (seq.chan_indirect[log_chan] != -1) {
        uint32_t idx = (uint8_t)seq.chan_indirect[log_chan];
        val = seq.ctrl_ptr[idx];
        seq.chan_indirect[log_chan] = -1;
    }

    if (ctrl == PART_VOLUME) {
        if (seq.vol_percent != 100) {
            uint32_t scaled = ((uint32_t)val * (uint32_t)seq.vol_percent) / 100;
            if (scaled > 127) scaled = 127;
            val = scaled;
        }
        uint32_t phys = seq.chan_map[log_chan];
        send_MIDI_message(0xB0 | (uint8_t)phys, ctrl, val);
        return 3;
    }

    if (ctrl == CLEAR_BEAT_BAR) {
        seq.beat_count = 0;
        seq.measure_count = 0;
        seq.beat_fraction = -(int32_t)seq.time_fraction;
        return 3;
    }

    if (ctrl == FOR_LOOP) {
        uint32_t slot;
        for (slot = 0; slot < XMI_FOR_NEST; slot++) {
            if (seq.FOR_loop_cnt[slot] == -1) break;
        }
        if (slot == XMI_FOR_NEST) return 3;
        seq.FOR_loop_cnt[slot] = (int16_t)val;
        seq.FOR_ptrs[slot] = seq.EVNT_ptr;
        return 3;
    }

    if (ctrl == NEXT_LOOP) {
        if (val < 64) return 3;
        uint32_t slot;
        for (slot = XMI_FOR_NEST; slot > 0; slot--) {
            if (seq.FOR_loop_cnt[slot - 1] != -1) break;
        }
        if (slot == 0) return 3;
        slot--;
        seq.FOR_loop_cnt[slot]--;
        if (seq.FOR_loop_cnt[slot] <= 0) {
            seq.FOR_loop_cnt[slot] = -1;
            return 3;
        }
        seq.EVNT_ptr = seq.FOR_ptrs[slot];
        return 3;
    }

    if (ctrl == INDIRECT_C_PFX) {
        seq.chan_indirect[log_chan] = (int8_t)val;
        return 3;
    }

    uint32_t phys = seq.chan_map[log_chan];
    send_MIDI_message(0xB0 | (uint8_t)phys, ctrl, val);
    return 3;
}

uint32_t CxmiPlayer::xmidi32_XMIDI_note_on() {
    const uint8_t *p = seq.EVNT_ptr;
    const uint8_t *event_start = p;

    uint8_t chan = p[0] & 0x0F;
    uint8_t note = p[1];
    uint8_t velocity = p[2];

    const uint8_t *dur_ptr = p + 3;
    uint32_t duration = 0;
    int shift_count = 0;

    for (;;) {
        uint8_t byte = *dur_ptr++;
        duration = (duration << 7) | (byte & 0x7F);
        shift_count++;
        if ((byte & 0x80) == 0) break;
        if (shift_count >= 4) break;
    }

    uint32_t event_len = (uint32_t)(dur_ptr - event_start);

    uint32_t phys_chan = seq.chan_map[chan];

    if (velocity == 0) {
        for (int ns = 0; ns < XMI_MAX_NOTES; ns++) {
            if (seq.note_queue[ns].chan == chan && seq.note_queue[ns].num == note) {
                seq.note_queue[ns].chan = 0xFF;
                if (seq.chan_map[chan & 0x0F] < XMI_NUM_CHANS)
                    yamaha_note_off(phys_chan, note);
                seq.note_count--;
                break;
            }
        }
        return event_len;
    }

    uint32_t slot;
    for (slot = 0; slot < XMI_MAX_NOTES; slot++) {
        if (seq.note_queue[slot].chan == 0xFF) break;
    }

    if (slot == XMI_MAX_NOTES) slot = 0;
    else seq.note_count++;

    seq.note_queue[slot].chan = (uint8_t)chan;
    seq.note_queue[slot].num = note;
    seq.note_queue[slot].time = (int32_t)duration - 1;

    send_MIDI_message(0x90 | phys_chan, note, velocity);

    return event_len;
}

uint32_t CxmiPlayer::xmidi32_XMIDI_meta() {
    const uint8_t *p = seq.EVNT_ptr;
    uint8_t meta_type = p[1];

    const uint8_t *data_start = p + 2;
    const uint8_t *vln_ptr = data_start;
    uint32_t data_len = read_vln(&vln_ptr);

    uint32_t header_len = (uint32_t)(vln_ptr - data_start);
    uint32_t total_len = header_len + data_len + 2;

    if (meta_type == 0x2F) {
        if (cur_track >= 0 && cur_track < 8 && has_loop[cur_track])
            seq.status = SEQ_DONE;
        else
            seq.status = SEQ_DONE;
        return total_len;
    }

    if (meta_type == 0x58) {
        seq.time_numerator = p[3];

        uint8_t denom_byte = p[4];
        uint8_t denom_exp;
        if (denom_byte >= 2) denom_exp = denom_byte - 2;
        else denom_exp = 2 - denom_byte;

        uint32_t base = XMI_QUANT_TIME_16 >> denom_exp;
        if (base == 0) base = 1;

        seq.time_fraction = (int32_t)base;
        seq.beat_fraction = -(int32_t)base;
        seq.beat_count = 0;
        seq.measure_count++;

        return total_len;
    }

    if (meta_type == 0x51) {
        if (data_len >= 3) {
            uint32_t tempo = ((uint32_t)p[3] << 16) |
                             ((uint32_t)p[4] << 8)  |
                             (uint32_t)p[5];
            tempo = (tempo << 4) & 0x0FFFFFF0U;
            seq.time_per_beat = tempo;
        }
        return total_len;
    }

    return total_len;
}

uint32_t CxmiPlayer::xmidi32_XMIDI_sysex() {
    const uint8_t *p = seq.EVNT_ptr;
    const uint8_t *data_start = p + 1;
    const uint8_t *vln_ptr = data_start;
    uint32_t data_len = read_vln(&vln_ptr);
    uint32_t total_len = (uint32_t)(vln_ptr - data_start) + data_len;

    return total_len;
}

void CxmiPlayer::xmidi32_XMIDI_volume() {
    for (uint32_t i = 0; i < XMI_NUM_CHANS; i++) {
        uint8_t pv = ((uint8_t *)&seq.chan_controls)[i];
        if (pv == 0xFF) continue;

        uint32_t vol = ((uint32_t)pv * (uint32_t)seq.vol_percent) / 100;
        if (vol > 127) vol = 127;

        uint32_t phys = seq.chan_map[i];
        send_MIDI_message(0xB0 | (uint8_t)phys, PART_VOLUME, vol);
    }
}

/* ======================== CPlayer interface ======================== */

bool CxmiPlayer::load(const std::string &filename, const CFileProvider &fp) {
    binistream *f = fp.open(filename);
    if (!f) return false;

    xmi_size = fp.filesize(f);
    if (xmi_size < 12) { fp.close(f); return false; }

    xmi_data = new uint8_t[xmi_size];
    f->readString((char *)xmi_data, xmi_size);
    fp.close(f);

    /* count tracks */
    num_tracks = 0;
    while (find_seq((uint32_t)num_tracks) != NULL) num_tracks++;

    if (num_tracks == 0) { delete[] xmi_data; xmi_data = 0; return false; }

    /* check for loops in each track */
    for (int t = 0; t < num_tracks && t < 8; t++) {
        uint8_t *form = find_seq((uint32_t)t);
        if (!form) { has_loop[t] = 0; continue; }
        uint8_t *form_end = form + 8 + read_be32(form + 4);
        uint8_t *p = form + 12;
        uint8_t *evnt = NULL;
        while (p + 8 <= form_end) {
            uint32_t tag = read_be32(p);
            uint32_t chunk_len = read_be32(p + 4);
            if (p + 8 + chunk_len > form_end) break;
            if (tag == 0x45564E54u) { evnt = p; break; } /* "EVNT" */
            p += 8 + chunk_len;
        }
        has_loop[t] = has_loops(evnt);
    }

    /* extract title/author from INFO chunk at the file root level
     * (INFO lives inside FORM XDIR, not inside any FORM XMID sequence) */
    {
        uint8_t *p = xmi_data;
        uint8_t *file_end = xmi_data + xmi_size;
        /* if the root chunk is FORM XDIR, walk its children */
        if (p + 12 <= file_end &&
            read_be32(p) == 0x464F524Du &&        /* "FORM" */
            read_be32(p + 8) == 0x58444952u)      /* "XDIR" */
        {
            uint8_t *dir_end = p + 8 + read_be32(p + 4);
            if (dir_end > file_end) dir_end = file_end;
            p += 12;
            while (p + 8 <= dir_end) {
                uint32_t tag      = read_be32(p);
                uint32_t clen     = read_be32(p + 4);
                if (p + 8 + clen > dir_end) break;
                if (tag == 0x494E464Fu) {      /* "INFO" */
                    const uint8_t *d = p + 8;
                    uint32_t remain = clen;
                    /* title */
                    if (remain > 2) {
                        uint32_t nlen = read_le_16(d);
                        d += 2; remain -= 2;
                        if (nlen > remain) nlen = remain;
                        if (nlen > 0) {
                            title_str = new uint8_t[nlen + 1];
                            memcpy(title_str, d, nlen);
                            title_str[nlen] = 0;
                            title_len = nlen;
                            d += nlen; remain -= nlen;
                        }
                    }
                    /* author */
                    if (remain > 2) {
                        uint32_t nlen = read_le_16(d);
                        d += 2; remain -= 2;
                        if (nlen > remain) nlen = remain;
                        if (nlen > 0) {
                            author_str = new uint8_t[nlen + 1];
                            memcpy(author_str, d, nlen);
                            author_str[nlen] = 0;
                            author_len = nlen;
                        }
                    }
                    break;
                }
                p += 8 + clen;
            }
        }
    }

    /* probe for an external GTL/OPL instrument bank in the same directory.
     * try each name in order; use the first one that opens successfully. */
    {
        static const char *const bank_names[] = {
            "MIDPAK.AD", "midpak.ad",
            "INSTR.AD", "instr.ad",
            "SAMPLE.AD", "sample.ad",
            "SAMPLE.OPL", "sample.opl",
            NULL
        };

        /* build the directory prefix from the XMI filename */
        std::string dir;
        size_t sep = filename.find_last_of("/\\");
        if (sep != std::string::npos)
            dir = filename.substr(0, sep + 1);

        for (int bi = 0; bank_names[bi] && !ext_bank_data; bi++) {
            std::string bank_path = dir + bank_names[bi];
            binistream *bf = fp.open(bank_path);
            if (!bf) continue;
            uint32_t bsz = fp.filesize(bf);
            if (bsz > 6) {   /* must hold at least one directory entry */
                ext_bank_data = new uint8_t[bsz];
                ext_bank_size = bsz;
                bf->readString((char *)ext_bank_data, bsz);
            }
            fp.close(bf);
        }
    }

    rewind(0);
    return true;
}

bool CxmiPlayer::update() {
    if (cur_track < 0 || cur_track >= num_tracks) return false;
    if (seq.status != SEQ_PLAYING) return false;

    /* process note queue */
    if (seq.note_count != 0) {
        for (int ns = 0; ns < XMI_MAX_NOTES; ns++) {
            if (seq.note_queue[ns].chan == 0xFF) continue;
            seq.note_queue[ns].time--;
            if (seq.note_queue[ns].time < 0) {
                uint8_t chan = seq.note_queue[ns].chan;
                uint8_t note = seq.note_queue[ns].num;
                seq.note_queue[ns].chan = 0xFF;
                if (seq.chan_map[chan & 0x0F] < XMI_NUM_CHANS)
                    yamaha_note_off(seq.chan_map[chan & 0x0F], note);
                seq.note_count--;
            }
        }
    }

    seq.interval_cnt--;
    if ((int16_t)seq.interval_cnt > 0) goto check_beat;

    while (1) {
        uint32_t status = seq.EVNT_ptr[0];

        if (status < 128) {
            seq.interval_cnt = (uint16_t)status;
            seq.EVNT_ptr++;
            goto check_beat;
        }

        uint32_t ctrl = status & 0xF0;
        uint32_t log_chan = status & 0x0F;
        uint8_t data1 = seq.EVNT_ptr[1];
        uint8_t data2 = seq.EVNT_ptr[2];

        if (ctrl == 0xF0) {
            if (log_chan == 0x0F) {
                uint32_t meta_len = xmidi32_XMIDI_meta();
                seq.EVNT_ptr += meta_len;
                if (seq.status != SEQ_PLAYING) return false;
            } else {
                uint32_t sysex_len = xmidi32_XMIDI_sysex();
                seq.EVNT_ptr += sysex_len;
            }
            goto next_event;
        }

        if (ctrl == 0xE0) {
            seq.chan_pitch_l[log_chan] = data1;
            seq.chan_pitch_h[log_chan] = data2;
            uint32_t phys_chan = seq.chan_map[log_chan];
            send_MIDI_message(0xE0 | phys_chan, data1, data2);
            seq.EVNT_ptr += 3;
            goto next_event;
        }

        if (ctrl == 0xD0) {
            uint32_t phys_chan = seq.chan_map[log_chan];
            send_MIDI_message(0xD0 | phys_chan, data1, 0);
            seq.EVNT_ptr += 2;
            goto next_event;
        }

        if (ctrl == 0xC0) {
            seq.chan_program[log_chan] = data1;
            uint32_t phys_chan = seq.chan_map[log_chan];
            send_MIDI_message(0xC0 | phys_chan, data1, 0);
            seq.EVNT_ptr += 2;
            goto next_event;
        }

        if (ctrl == 0xB0) {
            uint32_t ev_size = xmidi32_XMIDI_control(log_chan, data1, data2);
            seq.EVNT_ptr += ev_size;
            if (seq.status != SEQ_PLAYING) return false;
            goto next_event;
        }

        if (ctrl == 0xA0 || ctrl == 0x90) {
            uint32_t ev_size = xmidi32_XMIDI_note_on();
            seq.EVNT_ptr += ev_size;
            if (seq.status != SEQ_PLAYING) return false;
            goto next_event;
        }

        if (ctrl == 0x80) {
            uint32_t phys_chan = seq.chan_map[log_chan];
            for (int ns = 0; ns < XMI_MAX_NOTES; ns++) {
                if (seq.note_queue[ns].chan == log_chan && seq.note_queue[ns].num == data1) {
                    seq.note_queue[ns].chan = 0xFF;
                    if (seq.chan_map[log_chan & 0x0F] < XMI_NUM_CHANS)
                        yamaha_note_off(phys_chan, data1);
                    seq.note_count--;
                    break;
                }
            }
            seq.EVNT_ptr += 3;
            goto next_event;
        }

next_event:
        if (seq.status != SEQ_PLAYING) return false;
        continue;
    }

check_beat:
    {
        int32_t beat_frac = (int32_t)seq.beat_fraction + (int32_t)seq.time_fraction;
        if (beat_frac >= (int32_t)seq.time_per_beat) {
            seq.beat_fraction = beat_frac - (int32_t)seq.time_per_beat;
            seq.beat_count++;
            if (seq.beat_count >= seq.time_numerator) {
                seq.beat_count = 0;
                seq.measure_count++;
            }
        } else {
            seq.beat_fraction = beat_frac;
        }
    }

    return true;
}

void CxmiPlayer::rewind(int subsong) {
    if (subsong < 0) subsong = cur_track;
    if (subsong >= num_tracks) subsong = 0;

    cur_track = subsong;

    current_chip = -1;
    reset_yamaha();
    yamaha_define_cache();

    /* mirror xmidi32_init_driver(): send default controllers, pitch bend,
     * and default programs to all channels before registering the sequence.
     * This primes MIDI_program[] so that do_install_timbre() can immediately
     * associate freshly installed timbres with the correct channels. */
    for (uint32_t i = 0; i < XMI_NUM_CHANS; i++) {
        yamaha_controller(i, PART_VOLUME,  127);
        yamaha_controller(i, MODULATION,   0);
        yamaha_controller(i, PANPOT,       64);
        yamaha_controller(i, EXPRESSION,   127);
        yamaha_controller(i, SUSTAIN,      0);
        yamaha_pitch_bend(i, DEF_PITCH_L, DEF_PITCH_H);
    }

    /* default programs (matches xmidi32_globals.c prg_default[15]);
     * channel 8 (0xFF) is percussion — no default program change */
    static const uint8_t prg_default[15] = {
        68, 48, 95, 78, 41, 3, 110, 122, 0xFF, 0, 0, 0, 0, 0, 0
    };
    for (uint32_t i = 0; i < 15; i++) {
        if (prg_default[i] != 0xFF)
            yamaha_program_change(i, prg_default[i]);
    }

    rewind_seq();

    install_sequence_timbres();
}

float CxmiPlayer::getrefresh() {
    return (float)XMI_QUANT_RATE;
}

std::string CxmiPlayer::gettitle() {
    if (title_str && title_len > 0)
        return std::string((const char *)title_str, title_len);
    return std::string();
}

std::string CxmiPlayer::getauthor() {
    if (author_str && author_len > 0)
        return std::string((const char *)author_str, author_len);
    return std::string();
}

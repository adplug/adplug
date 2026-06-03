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
 * xmi.h - XMIDI (Extended MIDI) player for AdPlug
 *         Based on XMIDI32 by John Miles & John Lemberger, Miles Design, Inc.
 */

#ifndef H_ADPLUG_XMI
#define H_ADPLUG_XMI

#include "player.h"
#include <stdint.h>
#include <string.h>

#define XMI_NUM_CHANS      16
#define XMI_MAX_NOTES      32
#define XMI_FOR_NEST       4
#define XMI_NSEQS          8
#define XMI_MAX_TIMBS      192
#define XMI_CACHE_SIZE     12288
#define XMI_NUM_SLOTS      20
#define XMI_NUM_VOICES     18
#define XMI_NUM_4OP        6
#define XMI_QUANT_RATE     120

#define XMI_SLOT_FREE      0
#define XMI_SLOT_KEYON     1
#define XMI_SLOT_KEYOFF    2

#define XMI_BNK_INST       0
#define XMI_TV_INST        1
#define XMI_OPL3_INST      3

#define XMI_QUANT_TIME_16  0x0208D5U

struct xmi_ctrl_log {
    uint8_t PV[XMI_NUM_CHANS];
    uint8_t MODUL[XMI_NUM_CHANS];
    uint8_t PAN[XMI_NUM_CHANS];
    uint8_t EXP[XMI_NUM_CHANS];
    uint8_t SUS[XMI_NUM_CHANS];
    uint8_t PBS[XMI_NUM_CHANS];
    uint8_t C_LOCK[XMI_NUM_CHANS];
    uint8_t C_PROT[XMI_NUM_CHANS];
    uint8_t V_PROT[XMI_NUM_CHANS];
};

struct xmi_note_entry {
    uint8_t  chan;
    uint8_t  num;
    int32_t  time;
};

struct xmi_sequence {
    uint8_t *TIMB;
    uint8_t *RBRN;
    uint8_t *EVNT;
    uint8_t *EVNT_ptr;
    int32_t interval_cnt;
    uint16_t note_count;
    int32_t tempo_error;
    int32_t tempo_percent;
    int32_t tempo_target;
    int32_t tempo_accum;
    int32_t tempo_period;
    int32_t vol_error;
    int32_t vol_percent;
    int32_t vol_target;
    int32_t vol_accum;
    int32_t vol_period;
    uint16_t beat_count;
    int16_t  measure_count;
    uint16_t time_numerator;
    int32_t  time_fraction;
    int32_t  beat_fraction;
    int32_t  time_per_beat;
    uint8_t *FOR_ptrs[XMI_FOR_NEST];
    int16_t  FOR_loop_cnt[XMI_FOR_NEST];
    uint8_t chan_map[XMI_NUM_CHANS];
    uint8_t chan_program[XMI_NUM_CHANS];
    uint8_t chan_pitch_l[XMI_NUM_CHANS];
    uint8_t chan_pitch_h[XMI_NUM_CHANS];
    int8_t  chan_indirect[XMI_NUM_CHANS];
    uint8_t *ctrl_ptr;
    struct xmi_ctrl_log chan_controls;
    struct xmi_note_entry note_queue[XMI_MAX_NOTES];
    uint16_t seq_started;
    uint16_t status;
};

class CxmiPlayer : public CPlayer
{
public:
    static CPlayer *factory(Copl *newopl);

    CxmiPlayer(Copl *newopl);
    ~CxmiPlayer();

    bool load(const std::string &filename, const CFileProvider &fp);
    bool update();
    void rewind(int subsong);
    float getrefresh();

    std::string gettype() { return std::string("Miles Design eXtended MIDI"); }
    std::string gettitle();
    std::string getauthor();
    unsigned int getinstruments() { return (unsigned int)nrpn; }
    unsigned int getsubsongs() { return num_tracks; }
    unsigned int getsong() { return (unsigned int)cur_track; }

private:
    /* utility */
    static uint32_t read_be32(const uint8_t *p);
    static uint16_t read_le_16(const uint8_t *p);
    static uint32_t read_vln(const uint8_t **ptr);
    uint8_t *find_seq(uint32_t seq_num);
    void rewind_seq();

    /* sequence init */
    void init_sequence_state();

    /* event handlers */
    uint32_t xmidi32_XMIDI_control(uint32_t log_chan, uint32_t ctrl, uint32_t val);
    uint32_t xmidi32_XMIDI_note_on();
    uint32_t xmidi32_XMIDI_meta();
    uint32_t xmidi32_XMIDI_sysex();
    void xmidi32_XMIDI_volume();
    void flush_note_queue();
    void flush_channel_notes(uint32_t chan);

    /* OPL backend */
    void opl_write_chip(uint8_t chip, uint8_t reg, uint8_t val);
    void write_reg(uint8_t reg, uint8_t bank, uint8_t val);
    void update_reg(uint8_t oper, uint8_t base, uint8_t val);
    void send_byte(uint8_t voice, uint8_t base, uint8_t val);
    void reset_synth();
    void init_synth();
    void reset_yamaha();

    /* timbre cache */
    void yamaha_define_cache();
    uint32_t index_timbre(uint16_t gnum);
    void delete_LRU();
    void do_install_timbre(uint16_t gnum, const uint8_t *data);
    void yamaha_install_timbre(uint32_t bank, uint32_t patch, const uint8_t *data);
    void yamaha_protect_timbre(uint32_t bank, uint32_t patch);
    void yamaha_unprotect_timbre(uint32_t bank, uint32_t patch);
    int32_t yamaha_timbre_status(uint32_t bank, uint32_t patch);
    uint32_t timbre_request();
    int install_sequence_timbres();

    /* voice management */
    void release_voice(int32_t slot);
    void assign_voice(int32_t slot);
    void update_voice(int32_t slot);
    void update_priority();
    void BNK_phase(int32_t slot);
    void OPL_phase(int32_t slot);

    /* yamaha midi interface */
    void yamaha_note_on(uint32_t chan, uint32_t note, uint32_t vel);
    void yamaha_note_off(uint32_t chan, uint32_t note);
    void yamaha_controller(uint32_t chan, uint32_t ctrl, uint32_t val);
    void yamaha_program_change(uint32_t chan, uint32_t program);
    void yamaha_pitch_bend(uint32_t chan, uint32_t pitch_l, uint32_t pitch_h);
    void send_MIDI_message(uint32_t status, uint32_t d1, uint32_t d2);

    /* track info */
    int has_loops(uint8_t *evnt);

    /* static tables */
    static const uint16_t freq_table[192];
    static const uint8_t note_octave[128];
    static const uint8_t note_halftone[128];
    static const uint8_t note2_octave[96];
    static const uint8_t note2_htone[96];
    static const uint8_t array0_init[258];
    static const uint8_t array1_init[258];
    static const uint8_t vel_graph[16];
    static const uint8_t op_0[18];
    static const uint8_t op_1[18];
    static const uint8_t op_index[36];
    static const uint8_t op_array[36];
    static const uint8_t voice_num[18];
    static const uint8_t voice_array[18];
    static const uint8_t op4_base[18];
    static const int8_t alt_voice[18];
    static const uint8_t alt_op_0[18];
    static const uint8_t alt_op_1[18];
    static const uint8_t conn_sel[18];
    static const uint8_t op4_voice[6];
    static const uint8_t carrier_01_data[18];
    static const uint8_t carrier_23_data[18];

    /* file data */
    uint8_t *xmi_data;
    uint32_t xmi_size;
    int num_tracks;
    int cur_track;
    int has_loop[8];
    uint8_t *title_str;
    uint8_t *author_str;
    uint32_t title_len;
    uint32_t author_len;

    /* external GTL/OPL bank (SAMPLE.AD, SAMPLE.OPL, MIDPAK.AD, …) */
    uint8_t *ext_bank_data;
    uint32_t ext_bank_size;

    /* yamaha instance state */
    uint8_t *cache_mem;
    uint32_t cache_size;
    uint32_t cache_end;
    uint32_t note_event_ctr;
    uint32_t timb_hist[XMI_MAX_TIMBS];
    uint32_t timb_offsets[XMI_MAX_TIMBS];
    uint8_t  timb_bank[XMI_MAX_TIMBS];
    uint8_t  timb_num[XMI_MAX_TIMBS];
    uint8_t  timb_attribs[XMI_MAX_TIMBS];
    uint8_t  conn_shadow;
    int32_t  rover_2op;
    int32_t  rover_4op;
    int32_t  nrpn;

    /* slot state */
    uint8_t  S_timbre_off_l[XMI_NUM_SLOTS];
    uint8_t  S_timbre_off_h[XMI_NUM_SLOTS];
    uint16_t S_duration[XMI_NUM_SLOTS];
    uint8_t  S_status[XMI_NUM_SLOTS];
    uint8_t  S_type[XMI_NUM_SLOTS];
    int8_t   S_voice[XMI_NUM_SLOTS];
    uint8_t  S_channel[XMI_NUM_SLOTS];
    uint8_t  S_note[XMI_NUM_SLOTS];
    uint8_t  S_keynum[XMI_NUM_SLOTS];
    int8_t   S_transpose[XMI_NUM_SLOTS];
    uint8_t  S_velocity[XMI_NUM_SLOTS];
    uint8_t  S_sustain[XMI_NUM_SLOTS];
    uint8_t  S_update[XMI_NUM_SLOTS];
    uint8_t  S_KBF_shadow[XMI_NUM_SLOTS];
    uint8_t  S_BLOCK[XMI_NUM_SLOTS];
    uint8_t  S_FBC[XMI_NUM_SLOTS];
    uint8_t  S_KSLTL_0[XMI_NUM_SLOTS]; uint8_t  S_KSLTL_1[XMI_NUM_SLOTS];
    uint8_t  S_AVEKM_0[XMI_NUM_SLOTS]; uint8_t  S_AVEKM_1[XMI_NUM_SLOTS];
    uint8_t  S_AD_0[XMI_NUM_SLOTS];    uint8_t  S_AD_1[XMI_NUM_SLOTS];
    uint8_t  S_SR_0[XMI_NUM_SLOTS];    uint8_t  S_SR_1[XMI_NUM_SLOTS];
    uint8_t  S_scale_01[XMI_NUM_SLOTS];
    uint16_t S_ws_val[XMI_NUM_SLOTS];
    uint16_t S_m1_val[XMI_NUM_SLOTS];  uint16_t S_m0_val[XMI_NUM_SLOTS];
    uint16_t S_fb_val[XMI_NUM_SLOTS];
    uint16_t S_p_val[XMI_NUM_SLOTS];
    uint16_t S_v1_val[XMI_NUM_SLOTS];  uint16_t S_v0_val[XMI_NUM_SLOTS];
    uint8_t  S_KSLTL_2[XMI_NUM_SLOTS]; uint8_t  S_KSLTL_3[XMI_NUM_SLOTS];
    uint8_t  S_AVEKM_2[XMI_NUM_SLOTS]; uint8_t  S_AVEKM_3[XMI_NUM_SLOTS];
    uint8_t  S_AD_2[XMI_NUM_SLOTS];    uint8_t  S_AD_3[XMI_NUM_SLOTS];
    uint8_t  S_SR_2[XMI_NUM_SLOTS];    uint8_t  S_SR_3[XMI_NUM_SLOTS];
    uint8_t  S_scale_23[XMI_NUM_SLOTS];
    uint16_t S_ws_val_2[XMI_NUM_SLOTS];
    uint16_t S_m3_val[XMI_NUM_SLOTS];  uint16_t S_m2_val[XMI_NUM_SLOTS];
    uint16_t S_v3_val[XMI_NUM_SLOTS];  uint16_t S_v2_val[XMI_NUM_SLOTS];
    uint16_t S_V_priority[XMI_NUM_SLOTS];

    /* MIDI channel state */
    uint8_t  MIDI_vol[XMI_NUM_CHANS];
    uint8_t  MIDI_pan[XMI_NUM_CHANS];
    uint8_t  MIDI_pitch_l[XMI_NUM_CHANS];
    uint8_t  MIDI_pitch_h[XMI_NUM_CHANS];
    uint8_t  MIDI_express[XMI_NUM_CHANS];
    uint8_t  MIDI_mod[XMI_NUM_CHANS];
    uint8_t  MIDI_sus[XMI_NUM_CHANS];
    uint8_t  MIDI_vprot[XMI_NUM_CHANS];
    uint8_t  MIDI_bank[XMI_NUM_CHANS];
    int8_t   MIDI_timbre[XMI_NUM_CHANS];
    int8_t   MIDI_program[XMI_NUM_CHANS];
    uint8_t  MIDI_voices[XMI_NUM_CHANS];
    int8_t   RBS_timbres[128];
    int8_t   V_channel[XMI_NUM_VOICES];

    /* sequence state (current playing track) */
    struct xmi_sequence seq;
    int current_chip;
    static const uint8_t dummy_2op[14];
};

#endif

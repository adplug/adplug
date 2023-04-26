/*
  Adplug - Replayer for many OPL2/OPL3 audio file formats.
  Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  mtr.cpp - MTR loader by Dmitry Smagin <dmitry.s.smagin@gmail.com>
*/

#include <stdio.h>
#include <cstring>
#include "mtr.h"
#include "debug.h"

/* -------- Public Methods -------------------------------- */

CPlayer *CmtrLoader::factory(Copl *newopl) {
    return new CmtrLoader(newopl);
}

bool CmtrLoader::load(const std::string &filename, const CFileProvider &fp) {
    binistream *f = fp.open(filename);
    if (!f)
        return false;

    unsigned int i, j, k, t = 0;

    char header[51] = {0};
    int version, nvoices, ndigvoices, npatterns, orderlen,
        ninstruments = 0, speed = 0x428F, restart, flength;

    f->readString(header, 50);

    if (!strncmp(header, "MTRAC ", 6)) {
        version = 1;
        sscanf(header + 26, "%02x %02x %02x %02x %08x",
            &nvoices,
            &npatterns,
            &orderlen,
            &restart,
            &flength
        );
        speed = f->readInt(2); // usually 428F
        f->ignore(1); // 0=SPK 1=ADL 2=SBP
    } else if (!strncmp(header, "MTRACK NC", 9)) {
        version = 2;
        sscanf(header + 10, "%02x %02x %02x %02x %02x %02x %04x %08x",
            &nvoices,
            &ndigvoices,
            &npatterns,
            &orderlen,
            &ninstruments,
            &restart,
            &speed, // usually 428F
            &flength
        );
        f->ignore(20); // tune name
    } else {
        fp.close(f);
        return false;
    }

    // corrections for read data
    nvoices++;
    ninstruments = version == 2 ? ninstruments : 64;

    // data for Protracker
    length = orderlen + 1;
    nop = npatterns + 1;
    timer = 1193180 / (speed ? speed : 0x428F);

    // init CmodPlayer
    realloc_instruments(ninstruments);
    realloc_order(length);
    realloc_patterns(nop, 64, nvoices);
    init_trackord();

    // load order
    for (i = 0; i < length; i++)
        order[i] = f->readInt(1);
    f->ignore(256 - length);

    const unsigned char table[12] = {
        4, 0, 6, 2, 8, 3, 9, 5, 11, 1, 7
    };

    // read instruments
    for (i = 0; i < ninstruments; i++) {
        f->readString(instruments[i].name, 20);
        instruments[i].name[19] = 0;
        instruments[i].is_used = f->readInt(1);
        f->readString(instruments[i].data, 12);
        f->ignore(31);

        // convert
        if (instruments[i].is_used == 2) {
            for (j = 0; j < 11; j++) {
                inst[i].data[j] = instruments[i].data[table[j]];
            }
        }
    }

    // load tracks
    for (i = 0; i < nop; i++)
        for (k = 0; k < 64; k++) {
            for (j = 0; j < nvoices; j++) {
                char event[4], note, inst, fx, val;
                f->readString(event, 4);
                note = event[0] ? ((event[0] & 0xf) + ((event[0] >> 4) * 12)) : 0;
                inst = event[1] & 0x3f;
                fx = event[2] & 0xf;
                val = event[3];

                t = i * nvoices + j;
                tracks[t][k].note = note;
                tracks[t][k].inst = inst;

                // translate effects
                switch (fx) {
                case 0: // 0xy, arp
                // 1 and 2 never occur in any .mtr, so might sound wrong
                case 1: // 1xy, slide up
                case 2: // 2xy, slide down
                    tracks[t][k].command = fx;
                    tracks[t][k].param1 = val >> 4;
                    tracks[t][k].param2 = val & 0xf;
                    break;
                case 3: // 3xy, fine slide up
                case 4: // 4xy, fine slide down
                    tracks[t][k].command = fx == 3 ? 0x17 : 0x18;
                    tracks[t][k].param1 = val >> 4;
                    tracks[t][k].param2 = val & 0xf;
                    break;
                case 5:     // 5xy -> C(63-xy), set volume
                    tracks[t][k].command = 0xc;
                    tracks[t][k].param1 = (63 - val) >> 4;
                    tracks[t][k].param2 = (63 - val) & 0xf;
                    break;
                case 0xB:   // Bxy -> Fxy, set speed
                    tracks[t][k].command = 0xf;
                    tracks[t][k].param1 = val >> 4;
                    tracks[t][k].param2 = val & 0xf;
                    break;
                case 0xF:
                    if (val == 1) { // F01 -> D00, pattern break
                        tracks[t][k].command = 0xd;
                        tracks[t][k].param1 = 0;
                        tracks[t][k].param2 = 0;
                        break;
                    } else if (val == 2) { // F02 -> note off
                        tracks[t][k].note = 0x7f;
                        tracks[t][k].inst = 0;
                        break;
                    }
                default:
                    // Unsupported:
                    // Axy - retrigger
                    // Cxy - go to order position
                    // F00 - stop playing and restart
                    if (fx | val)
                        AdPlug_LogWrite("Unsupported effect: %02x-%02x\n", fx, val);
                }
            }
        }

    fp.close(f);

    // data for Protracker
    restartpos = restart;
    initspeed = 6;

    rewind(0);
    return true;
}

void CmtrLoader::rewind(int subsong) {
    CmodPlayer::rewind(subsong);
}

float CmtrLoader::getrefresh() {
    return (float)timer;
}

std::string CmtrLoader::gettype() {
    return std::string("Master Tracker");
}

std::string CmtrLoader::getinstrument(unsigned int n) {
    return std::string(instruments[n].name, 20);
}

unsigned int CmtrLoader::getinstruments() {
    return 9;
}

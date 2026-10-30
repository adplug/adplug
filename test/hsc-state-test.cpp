/*
 * AdPlug - HSC player reset-state regression test.
 * Copyright (C) 2026 dajiaohuang
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 */

#include <cstring>
#include <iostream>
#include <utility>
#include <vector>

#include "../src/hsc.h"

class RecordingOpl final : public Copl {
 public:
  void write(int reg, int value) override { writes.emplace_back(reg, value); }
  void init() override { writes.clear(); }

  int lastValue(int reg) const {
    for (auto it = writes.rbegin(); it != writes.rend(); ++it) {
      if (it->first == reg) return it->second;
    }
    return -1;
  }

 private:
  std::vector<std::pair<int, int>> writes;
};

class HscStateProbe final : public ChscPlayer {
 public:
  using ChscPlayer::ChscPlayer;

  void prepareSyntheticPattern(unsigned char note, unsigned char effect) {
    std::memset(instr, 0, sizeof(instr));
    std::memset(song, 0xff, sizeof(song));
    song[0] = 0;
    std::memset(patterns, 0, sizeof(patterns));
    patterns[0][0].note = note;
    patterns[0][0].effect = effect;
  }

  void poisonRuntimeState() {
    for (int i = 0; i < 9; ++i) {
      channel[i].slide = 7;
      channel[i].freq = 1234;
      adl_freq[i] = 0xa5;
    }
  }

  bool runtimeStateIsReset() const {
    for (int i = 0; i < 9; ++i) {
      if (channel[i].slide != 0 || channel[i].freq != 0 || adl_freq[i] != 0)
        return false;
    }
    return true;
  }

  int slide(int channelNumber) const { return channel[channelNumber].slide; }
  unsigned short frequency(int channelNumber) const {
    return channel[channelNumber].freq;
  }
};

int main() {
  RecordingOpl opl;

  // Synthetic in-memory state only: no song file, audio output or hardware.
  HscStateProbe slidingPlayer(&opl);
  slidingPlayer.prepareSyntheticPattern(0, 0x12);  // slide up by two
  slidingPlayer.poisonRuntimeState();
  slidingPlayer.rewind(0);
  if (!slidingPlayer.runtimeStateIsReset()) {
    std::cerr << "rewind left HSC channel state uninitialized\n";
    return 1;
  }
  slidingPlayer.update();
  if (slidingPlayer.slide(0) != 2 || slidingPlayer.frequency(0) != 2 ||
      opl.lastValue(0xa0) != 2 || opl.lastValue(0xb0) != 0) {
    std::cerr << "first slide used stale HSC channel state\n";
    return 1;
  }

  HscStateProbe noteOffPlayer(&opl);
  noteOffPlayer.prepareSyntheticPattern(0x7f, 0);  // pause before any note
  noteOffPlayer.poisonRuntimeState();
  noteOffPlayer.rewind(0);
  noteOffPlayer.update();
  if (opl.lastValue(0xb0) != 0) {
    std::cerr << "first note-off emitted stale HSC frequency bits\n";
    return 1;
  }

  return 0;
}

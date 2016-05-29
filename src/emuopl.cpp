/*
 * AdPlug - Replayer for many OPL2/OPL3 audio file formats.
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
 * emuopl.cpp - Emulated OPL, by Simon Peter <dn.tlp@gmx.net>
 */

#include "emuopl.h"

CEmuopl::CEmuopl(int rate, bool bit16, bool usestereo)
  : use16bit(bit16), stereo(usestereo), mixbufSamples(0)
{
  opl[0] = YM3812Init(3579545, rate);
  opl[1] = YM3812Init(3579545, rate);
  opl3 = new YMF262::Class();
  opl3->YMF262Init(1, 14400000, rate);

  currType = TYPE_OPL3;

  init();
}

CEmuopl::~CEmuopl()
{
  YM3812Shutdown(opl[0]); YM3812Shutdown(opl[1]);
  opl3->YMF262Shutdown(); delete opl3;

  if(mixbufSamples) {
    delete [] mixbuf0;
    delete [] mixbuf1;
  }
}

void CEmuopl::update(short *buf, int samples)
{
  int i;

  //ensure that our mix buffers are adequately sized
  if(mixbufSamples < samples) {
    if(mixbufSamples) { delete[] mixbuf0; delete[] mixbuf1; }
    mixbufSamples = samples;

    //*2 = make room for stereo, if we need it
    mixbuf0 = new short[samples*2]; 
    mixbuf1 = new short[samples*2];
  }

  //data should be rendered to outbuf
  //tempbuf should be used as a temporary buffer
  //if we are supposed to generate 16bit output,
  //then outbuf may point directly to the actual waveform output "buf"
  //if we are supposed to generate 8bit output,
  //then outbuf cannot point to "buf" (because there will not be enough room)
  //and so it must point to a mixbuf instead--
  //it will be reduced to 8bit and put in "buf" later
  short *outbuf;
  short *tempbuf=mixbuf0;
  short *tempbuf2=mixbuf1;
  if(use16bit) outbuf = buf;
  else outbuf = mixbuf1;
  //...there is a potentially confusing situation where mixbuf1 can be aliased.
  //beware. it is a little loony.

  //all of the following rendering code produces 16bit output

  switch(currType) {
  case TYPE_OPL2:
    //for opl2 mode:
    //render chip0 to the output buffer
    YM3812UpdateOne(opl[0],outbuf,samples);

    //if we are supposed to output stereo,
    //then we need to dup the mono channel
    if(stereo)
      for(i=samples-1;i>=0;i--) {
	outbuf[i*2] = outbuf[i];
	outbuf[i*2+1] = outbuf[i];
      }
    break;

  case TYPE_OPL3:
    //for opl3 mode:
    //if we are to render in stereo, render straight to outbuf
    if(stereo) {
      opl3->YMF262UpdateOne(0,outbuf,samples);
    } else {
    //otherwise, render to a tempbuf and then we will combine the channels
      opl3->YMF262UpdateOne(0,tempbuf,samples);

      for(i=0;i<samples;i++)
	outbuf[i] = (tempbuf[i*2]>>1) + (tempbuf[i*2+1]>>1);
    }
    break;

  case TYPE_DUAL_OPL2:
    //for dual opl2 mode:
    //render each chip to a different tempbuffer
    YM3812UpdateOne(opl[0],tempbuf2,samples);
    YM3812UpdateOne(opl[1],tempbuf,samples);

    //output stereo:
    //then we need to interleave the two buffers
    if(stereo){
      //first, spread tempbuf's samples across left channel
      //left channel
      for(i=0;i<samples;i++)
	outbuf[i*2] = tempbuf2[i];
      //next, insert the samples from tempbuf2 into right channel
      for(i=0;i<samples;i++)
	outbuf[i*2+1] = tempbuf[i];
    } else
      //output mono:
      //then we need to mix the two buffers into buf
      for(i=0;i<samples;i++)
	outbuf[i] = (tempbuf[i]>>1) + (tempbuf2[i]>>1);
    break;
  }

  //now reduce to 8bit if we need to
  if(!use16bit)
    for(i=0;i<(stereo ? samples*2 : samples);i++)
      ((char *)buf)[i] = (outbuf[i] >> 8) ^ 0x80;
}

void CEmuopl::write(int reg, int val)
{
  switch(currType){
  case TYPE_OPL2:
  case TYPE_DUAL_OPL2:
    YM3812Write(opl[currChip], 0, reg);
    YM3812Write(opl[currChip], 1, val);
    break;
  case TYPE_OPL3:
    opl3->YMF262Write(0, currChip * 2 + 0, reg);
    opl3->YMF262Write(0, currChip * 2 + 1, val);
    break;
  }
}

void CEmuopl::init()
{
  YM3812ResetChip(opl[0]); YM3812ResetChip(opl[1]);
  opl3->YMF262ResetChip(0);
  currChip = 0;
}

void CEmuopl::settype(ChipType type)
{
  currType = type;
}

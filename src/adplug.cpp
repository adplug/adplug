/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2002 Simon Peter <dn.tlp@gmx.net>, et al.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * adplug.cpp - CAdPlug helper class implementation, by Simon Peter <dn.tlp@gmx.net>
 */

/***** Includes *****/

#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string>

#include "adplug.h"
#include "debug.h"

/***** Replayer includes *****/

#include "hsc.h"
#include "mtk.h"
#include "hsp.h"
#include "s3m.h"
#include "raw.h"
#include "d00.h"
#include "sa2.h"
#include "amd.h"
#include "rad.h"
#include "a2m.h"
#include "mid.h"
#include "imf.h"
#include "sng.h"
#include "ksm.h"
#include "mkj.h"
#include "dfm.h"
#include "lds.h"
#include "bam.h"
#include "fmc.h"
#include "bmf.h"
#include "flash.h"
#include "hyp.h"
#include "psi.h"
#include "rat.h"
#include "hybrid.h"
#include "mad.h"
#include "adtrack.h"
#include "cff.h"

// These players use C++ templates, which aren't supported by WATCOM C++
#ifndef __WATCOMC__
#include "u6m.h"
#include "rol.h"
#endif

/***** Defines *****/

#define VERSION		"1.3"		// AdPlug library version string

/***** Static variables initializers *****/

const unsigned short CPlayer::note_table[12] = {363,385,408,432,458,485,514,544,577,611,647,686};
const unsigned char CPlayer::op_table[9] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

/***** List of all players that come in the standard AdPlug distribution *****/

// WARNING: The order of this list is on purpose! AdPlug tries the players in
// this order, which is to be preserved, because some players take precedence
// above other players.
// The list is terminated with an all-NULL element.
static const struct Players {
  CPlayer *(*factory) (Copl *newopl);
} allplayers[] = {
  {CmidPlayer::factory}, {CksmPlayer::factory},
#ifndef __WATCOMC__
  {CrolPlayer::factory},
#endif
  {CsngPlayer::factory}, {Ca2mLoader::factory}, {CradLoader::factory},
  {CamdLoader::factory}, {Csa2Loader::factory}, {CrawPlayer::factory},
  {Cs3mPlayer::factory}, {CmtkLoader::factory}, {CmkjPlayer::factory},
  {CdfmLoader::factory}, {CbamPlayer::factory}, {CxadbmfPlayer::factory},
  {CxadflashPlayer::factory}, {CxadhypPlayer::factory}, {CxadpsiPlayer::factory},
  {CxadratPlayer::factory}, {CxadhybridPlayer::factory}, {CfmcLoader::factory},
  {CmadLoader::factory}, {CcffLoader::factory},
#ifndef __WATCOMC__
  {Cu6mPlayer::factory},
#endif
  {Cd00Player::factory}, {ChspLoader::factory}, {ChscPlayer::factory},
  {CimfPlayer::factory}, {CldsLoader::factory}, {CadtrackLoader::factory},
  {0}
};

/***** Public methods *****/

CPlayer *CAdPlug::factory(const char *fn, Copl *opl)
{
	ifstream f(fn, ios::in | ios::binary);

        if(!f.is_open()) {
                AdPlug_LogWrite("CAdPlug::factory(\"%s\",opl): File could not be "
                        "opened!\n",fn);
                return 0;
        }
	return factory(f,opl,fn);
}

CPlayer *CAdPlug::factory(istream &f, Copl *opl, const char *fn)
{
  CPlayer *p;
  unsigned int i;

  AdPlug_LogWrite("*** CAdPlug::factory(f,opl,\"%s\") ***\n",fn);

  for(i=0;allplayers[i].factory;i++) {
    AdPlug_LogWrite("Trying: %d\n",i);
    if((p = allplayers[i].factory(opl)))
      if(p->load(f,fn)) {
        AdPlug_LogWrite("got it!\n");
        AdPlug_LogWrite("--- CAdPlug::factory ---\n");
	return p;
      } else {
	delete p;
	f.seekg(0);
      }
  }

  AdPlug_LogWrite("End of list!\n");
  AdPlug_LogWrite("--- CAdPlug::factory ---\n");
  return 0;
}

unsigned long CAdPlug::songlength(CPlayer *p, unsigned int subsong)
{
	float	slength = 0.0f;

	// get song length
	p->rewind(subsong);
	while(p->update() && slength < 600000)	// song length limit: 10 minutes
		slength += 1000/p->getrefresh();
	p->rewind(subsong);

	return (unsigned long)slength;
}

void CAdPlug::seek(CPlayer *p, unsigned long ms)
{
	float pos = 0.0f;

	p->rewind();
	while(pos < ms && p->update())		// seek to new position
		pos += 1000/p->getrefresh();
}

std::string CAdPlug::get_version()
{
  return std::string(VERSION);
}

void CAdPlug::debug_output(std::string filename)
{
  AdPlug_LogFile(filename.c_str());
  AdPlug_LogWrite("CAdPlug::debug_output(\"%s\"): Redirected.\n",filename.c_str());
}

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
 * adplug.cpp - CAdPlug utility class, by Simon Peter <dn.tlp@gmx.net>
 */

#include <string>
#include <binfile.h>

#include "adplug.h"
#include "debug.h"

/***** Replayer includes *****/

#include "hsc.h"
#include "amd.h"
#include "a2m.h"
#include "imf.h"
#include "sng.h"
#include "adtrack.h"
/*#include "mtk.h"
#include "hsp.h"
#include "s3m.h"
#include "raw.h"
#include "d00.h"
#include "sa2.h"
#include "rad.h"
#include "mid.h"
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
#include "cff.h"
#include "dtm.h"
#include "dmo.h"
#include "u6m.h"
#include "rol.h" */

/***** Defines *****/

#define VERSION		"1.4"		// AdPlug library version string

/***** Static stuff *****/

// List of all players that come with the standard AdPlug distribution
static const CPlayerDesc allplayers[] = {
  CPlayerDesc(ChscPlayer::factory, "HSC-Tracker", ".hsc"),
  CPlayerDesc(CsngPlayer::factory, "SNGPlay", ".sng"),
  CPlayerDesc(CimfPlayer::factory, "IMF", ".imf"),
  CPlayerDesc(Ca2mLoader::factory, "Adlib Tracker 2", ".a2m"),
  CPlayerDesc(CadtrackLoader::factory, "Adlib Tracker 1.0", ".sng"),
  CPlayerDesc(CamdLoader::factory, "AMUSIC AdLib Tracker", ".amd"),
  CPlayerDesc()
};

static const CPlayers &init_players(const CPlayerDesc pd[])
{
  static CPlayers	initplayers;
  unsigned int		i;

  for(i = 0; pd[i].factory; i++)
    initplayers.push_back(&pd[i]);

  return initplayers;
}

/***** CAdPlug *****/

const CPlayers CAdPlug::players = init_players(allplayers);
CAdPlugDatabase *CAdPlug::database = 0;

CPlayer *CAdPlug::factory(const std::string &fn, Copl *opl, const CPlayers &pl,
			  const CFileProvider &fp)
{
  CPlayer			*p;
  CPlayers::const_iterator	i;

  AdPlug_LogWrite("*** CAdPlug::factory(\"%s\",opl,fp) ***\n", fn.c_str());

  for(i = pl.begin(); i != pl.end(); i++) {
    AdPlug_LogWrite("Trying: %s\n", (*i)->filetype.c_str());
    if((p = (*i)->factory(opl)))
      if(p->load(fn, fp)) {
        AdPlug_LogWrite("got it!\n");
        AdPlug_LogWrite("--- CAdPlug::factory ---\n");
	return p;
      } else
	delete p;
  }

  AdPlug_LogWrite("End of list!\n");
  AdPlug_LogWrite("--- CAdPlug::factory ---\n");
  return 0;
}

void CAdPlug::set_database(CAdPlugDatabase *db)
{
  database = db;
}

std::string CAdPlug::get_version()
{
  return std::string(VERSION);
}

void CAdPlug::debug_output(const std::string &filename)
{
  AdPlug_LogFile(filename.c_str());
  AdPlug_LogWrite("CAdPlug::debug_output(\"%s\"): Redirected.\n",filename.c_str());
}

/**************************/

// WARNING: The order of this list is on purpose! AdPlug tries the players in
// this order, which is to be preserved, because some players take precedence
// above other players.
// The list is terminated with an all-NULL element.
/*const CAdPlug::Players CAdPlug::allplayers[] = {
  {CmidPlayer::factory}, {CksmPlayer::factory}, {CrolPlayer::factory},
  {CsngPlayer::factory}, {Ca2mLoader::factory}, {CradLoader::factory},
  {CamdLoader::factory}, {Csa2Loader::factory}, {CrawPlayer::factory},
  {Cs3mPlayer::factory}, {CmtkLoader::factory}, {CmkjPlayer::factory},
  {CdfmLoader::factory}, {CbamPlayer::factory}, {CxadbmfPlayer::factory},
  {CxadflashPlayer::factory}, {CxadhypPlayer::factory}, {CxadpsiPlayer::factory},
  {CxadratPlayer::factory}, {CxadhybridPlayer::factory}, {CfmcLoader::factory},
  {CmadLoader::factory}, {CcffLoader::factory}, {CdtmLoader::factory},
  {CdmoLoader::factory}, {Cu6mPlayer::factory}, {Cd00Player::factory},
  {ChspLoader::factory}, {ChscPlayer::factory}, {CimfPlayer::factory},
  {CldsLoader::factory}, {CadtrackLoader::factory},

  {ChscPlayer::factory, CFileType::HSCTracker, ".hsc", "HSC-Tracker", true},
  {CsngPlayer::factory, CFileType::SNGPlay, ".sng", "SNGPlay", true},
  {CimfPlayer::factory, CFileType::IMF, ".imf", "Apogee IMF", true},
  {0}
}; */

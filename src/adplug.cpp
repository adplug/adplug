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
 *
 * adplug.cpp - CAdPlug helper class implementation, by Simon Peter <dn.tlp@gmx.net>
 */

#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "adplug.h"

// replayers
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
//#include "lds.h"
#include "bam.h"
#include "fmc.h"
#include "bmf.h"
#include "flash.h"
#include "hyp.h"
#include "psi.h"
#include "rat.h"
#include "hybrid.h"
#include "mad.h"
#ifndef __WATCOMC__
	#include "u6m.h"
	#include "rol.h"
#endif

const unsigned short CPlayer::note_table[12] = {363,385,408,432,458,485,514,544,577,611,647,686};
const unsigned char CPlayer::op_table[9] = {0x00, 0x01, 0x02, 0x08, 0x09, 0x0a, 0x10, 0x11, 0x12};

int CAdPlug::get_basepath_index(const char *fn)
{
  int i;

  for (i=strlen(fn)-1; i>=0; i--)
    if (fn[i] == '/' || fn[i] == '\\')
      break;

  return ++i;
}

CPlayer *CAdPlug::load_sci(istream &f, const char *fn, Copl *opl)
{
	CmidPlayer *mp = new CmidPlayer(opl);
	char *pfn = new char [strlen(fn)+9];

	strcpy(pfn,fn);
	strcpy(pfn+get_basepath_index(fn)+3,"patch.003");
	mp->set_sierra_insfile(pfn);
	delete [] pfn;

	if(mp->load(f))
		return mp;
	delete mp;
	f.seekg(0);
	return 0;
}

CPlayer *CAdPlug::load_ksm(istream &f, const char *fn, Copl *opl)
{
	CksmPlayer	*mp = new CksmPlayer(opl);
	char		*pfn = new char [strlen(fn)+9];

	strcpy(pfn,fn);
	strcpy(pfn+get_basepath_index(fn),"insts.dat");
	ifstream insf(pfn, ios::in | ios::binary);
	delete [] pfn;
	if(!insf.is_open())
		return 0;
	mp->loadinsts(insf);
	if(mp->load(f))
		return mp;
	delete mp;
	f.seekg(0);
	return 0;
}

CPlayer *CAdPlug::load_rol(istream &f, const char *fn, Copl *opl)
{
#ifndef __WATCOMC__
	CrolPlayer	*mp = new CrolPlayer(opl);
	char		*pfn = new char [strlen(fn)+9];

	strcpy(pfn,fn);
	strcpy(pfn+get_basepath_index(fn),"standard.bnk");
	mp->get_bnk_filename(std::string(pfn));
	delete [] pfn;
	if(mp->load(f))
		return mp;
	delete mp;
	f.seekg(0);
#endif
	return 0;
}

CPlayer *CAdPlug::factory(const char *fn, Copl *opl)
{
	CPlayer		*p;
	ifstream	f(fn, ios::in | ios::binary);

	if(f.is_open()) {
		if(!stricmp(strrchr(fn,'.')+1,"SCI"))
			return load_sci(f,fn,opl);

		if(!stricmp(strrchr(fn,'.')+1,"KSM"))
			return load_ksm(f,fn,opl);

#ifndef __WATCOMC__
		if(!stricmp(strrchr(fn,'.')+1,"ROL"))
			return load_rol(f,fn,opl);
#endif

		if((p = factory(f,opl)))
			return p;

#ifndef __WATCOMC__
		if(!stricmp(strrchr(fn,'.')+1,"M")) {
			p = new Cu6mPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
#endif
		if(!stricmp(strrchr(fn,'.')+1,"D00")) {
			p = new Cd00Player(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
		if(!stricmp(strrchr(fn,'.')+1,"HSP")) {
			p = new ChspLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
		if(!stricmp(strrchr(fn,'.')+1,"HSC")) {
			p = new ChscPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
		if(!stricmp(strrchr(fn,'.')+1,"IMF") || !stricmp(strrchr(fn,'.')+1,"WLF")) {
			p = new CimfPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
		if(!stricmp(strrchr(fn,'.')+1,"KSM")) {
			p = new CksmPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
/*		if(!stricmp(strrchr(fn,'.')+1,"LDS")) {
			p = new CldsLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		} */
	};

	return 0;
}

CPlayer *CAdPlug::factory(istream &f, Copl *opl)
{
	CPlayer *p;

	p = new CsngPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CmidPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new Ca2mLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CradLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CamdLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new Csa2Loader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CrawPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new Cs3mPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CmtkLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CmkjPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CdfmLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CbamPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CxadbmfPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CxadflashPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CxadhypPlayer(opl);  if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CxadpsiPlayer(opl);  if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CxadratPlayer(opl);  if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CxadhybridPlayer(opl);  if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CfmcLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
	p = new CmadLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);

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

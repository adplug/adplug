/*
 * adplug.cpp - CAdPlug main class implementation, by Simon Peter (dn.tlp@gmx.net)
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
#ifndef ADPLAY
	#include "u6m.h"
#endif
#include "mkj.h"

char *CAdPlug::upstr(char *str)
{
	unsigned int i;

	for(i=0;i<strlen(str);i++)
		str[i] = toupper(str[i]);

	return str;
}

CPlayer *CAdPlug::load_sci(istream &f, char *fn, Copl *opl)
{
	CmidPlayer *mp = new CmidPlayer(opl);
	char *pfn = new char [strlen(fn)+9];
	int j=0;

	strcpy(pfn,fn);
	for (int i=strlen(pfn)-1; i>=0; i--)
		if (pfn[i]=='/' || pfn[i]=='\\') {
			j=i+1;
			i=-1;
		}
	strcpy(pfn+j+3,"patch.003");
	mp->set_sierra_insfile(pfn);
	delete [] pfn;

	if(mp->load(f))
		return mp;
	delete mp;
	f.seekg(0);
	return 0;
}

CPlayer *CAdPlug::load_ksm(istream &f, char *fn, Copl *opl)
{
	CksmPlayer	*mp = new CksmPlayer(opl);
	char		*pfn = new char [strlen(fn)+9];
	int			j=0;

	strcpy(pfn,fn);
	for (int i=strlen(pfn)-1; i>=0; i--)
		if (pfn[i]=='/' || pfn[i]=='\\') {
			j=i+1;
			i=-1;
		}
	strcpy(pfn+j,"insts.dat");
	ifstream insf(pfn, ios::in | ios::nocreate | ios::binary);
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

CPlayer *CAdPlug::factory(char *fn, Copl *opl)
{
	CPlayer		*p;
	ifstream	f(fn, ios::in | ios::nocreate | ios::binary);

	if(f.is_open()) {
		if(!strcmp(upstr(strrchr(fn,'.')+1),"SCI"))
			return load_sci(f,fn,opl);

		if(!strcmp(upstr(strrchr(fn,'.')+1),"KSM"))
			return load_ksm(f,fn,opl);

		if(p = factory(f,opl))
			return p;

#ifndef ADPLAY
		if(!strcmp(upstr(strrchr(fn,'.')+1),"M")) {
			p = new Cu6mPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
#endif
		if(!strcmp(upstr(strrchr(fn,'.')+1),"D00")) {
			p = new Cd00Player(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
		if(!strcmp(upstr(strrchr(fn,'.')+1),"HSP")) {
			p = new ChspLoader(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
		if(!strcmp(upstr(strrchr(fn,'.')+1),"HSC")) {
			p = new ChscPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
		if(!strcmp(upstr(strrchr(fn,'.')+1),"IMF") || !strcmp(upstr(strrchr(fn,'.')+1),"WLF")) {
			p = new CimfPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
		if(!strcmp(upstr(strrchr(fn,'.')+1),"KSM")) {
			p = new CksmPlayer(opl); if(p->load(f)) return p; delete p; f.seekg(0);
		}
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

/*
 * kemuopl.h - Emulated OPL using Ken Silverman's emulator, by Simon Peter (dn.tlp@gmx.net)
 */

#include "opl.h"
extern "C" {
#include "adlibemu.h"
}

class CKemuopl: public Copl
{
public:
	CKemuopl(int rate, bool bit16, bool usestereo)
		: use16bit(bit16), stereo(usestereo)
	{
		adlib=adlibinit(rate,usestereo ? 2 : 1,bit16 ? 2 : 1);
	};

	~CKemuopl()
	{
		adlibshutdown(adlib);
	};

	void update(short *buf, int samples)
	{
		adlibgetsample(adlib,buf,samples);
	};

	// template methods
	void write(int reg, int val)
	{
		adlibwrite(adlib,reg,val);
	};
	void init()
	{
		adlibreset(adlib);
	};

private:
	bool use16bit,stereo;
	ADLIB_STATE *adlib;
};

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
		adlibinit(rate,usestereo ? 2 : 1,bit16 ? 2 : 1);
	};

	void update(short *buf, int samples)
	{
		if(use16bit) samples *= 2;
		if(stereo) samples *= 2;
		adlibgetsample(buf,samples);
	}

	// template methods
	void write(int reg, int val)
	{
		adlib0(reg,val);
	};
	void init()
	{ };

private:
	bool use16bit,stereo;
};

/*
 * emuopl.h - Emulated OPL, by Simon Peter (dn.tlp@gmx.net)
 */

#include "opl.h"
extern "C" {
#include "fmopl.h"
}

class CEmuopl: public Copl
{
public:
	CEmuopl(int rate, bool bit16, bool usestereo)	// rate = sample rate
		: use16bit(bit16), stereo(usestereo)
	{
		opl = OPLCreate(OPL_TYPE_YM3812,3579545,rate);
	};
	~CEmuopl()
	{
		OPLDestroy(opl);
	};

	void update(short *buf, int samples);	// fill buffer

	// template methods
	void write(int reg, int val)
	{
		OPLWrite(opl,0,reg);
		OPLWrite(opl,1,val);
	};
	void init()
	{ };

private:
	bool use16bit,stereo;

	FM_OPL	*opl;			// holds emulator data
};

/*
 * silentopl.h - Silent OPL device, by Simon Peter (dn.tlp@gmx.net)
 */

#include "opl.h"

class CSilentopl: public Copl
{
public:
	void write(int reg, int val) { };
	void init() { };
};

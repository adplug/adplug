/*
  [xad] HYP player, by Riven the Mage <riven@ok.ru>
*/

#include "xad.h"

class CxadhypPlayer: public CxadPlayer
{
public:
  static CPlayer *factory(Copl *newopl);

  CxadhypPlayer(Copl *newopl): CxadPlayer(newopl)
    { }

protected:
  struct
  {
    unsigned short  pointer;
  } hyp;
  //
  bool		    xadplayer_load(istream &f)
    {
      if(xad.fmt == HYP)
	return true;
      else
	return false;
    }
  void 		    xadplayer_rewind(unsigned int subsong);
  void 		    xadplayer_update();
  float 	    xadplayer_getrefresh();
  std::string	    xadplayer_gettype();
};

/*
  [xad] FLASH player, by Riven the Mage <riven@ok.ru>
*/

#include "xad.h"

class CxadflashPlayer: public CxadPlayer
{
public:
  CxadflashPlayer(Copl *newopl): CxadPlayer(newopl)
    { };

protected:
  struct
  {
    unsigned char   order_pos;
    unsigned char   pattern_pos;
  } flash;
  //
  bool		  xadplayer_load(istream &f)
    {
      if(xad.fmt == FLASH)
	return true;
      else
	return false;
    }
  void            xadplayer_rewind(unsigned int subsong);
  void            xadplayer_update();
  float           xadplayer_getrefresh();
  std::string     xadplayer_gettype();
  unsigned int    xadplayer_getinstruments();
};

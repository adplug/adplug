/*
  [xad] PSI player, by Riven the Mage <riven@ok.ru>
*/

#include "xad.h"

class CxadpsiPlayer: public CxadPlayer
{
public:
  CxadpsiPlayer(Copl *newopl): CxadPlayer(newopl)
    { }

protected:
  struct psi_header
  {
    unsigned short  instr_ptr;
    unsigned short  seq_ptr;
  };

  struct
  {
    unsigned short  *instr_table;
    unsigned short  *seq_table;
    unsigned char   note_delay[9];
    unsigned char   note_curdelay[9];
    unsigned char   looping[9];
  } psi;
  //
  bool		  xadplayer_load(istream &f)
    {
      if(xad.fmt == PSI)
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

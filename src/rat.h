/*
  [xad] RAT player, by Riven the Mage <riven@ok.ru>
*/

#include "xad.h"

class CxadratPlayer: public CxadPlayer
{
public:
  static CPlayer *factory(Copl *newopl);

  CxadratPlayer(Copl *newopl): CxadPlayer(newopl)
    { }

protected:
  struct rat_header
  {
    char            id[3];
    unsigned char   version;
    char            title[32];
    unsigned char   numchan;
    unsigned char   reserved_25;
    unsigned char   order_end;
    unsigned char   reserved_27;
    unsigned char   numinst;              // ?: Number of Instruments
    unsigned char   reserved_29;
    unsigned char   numpat;               // ?: Number of Patterns
    unsigned char   reserved_2B;
    unsigned char   order_start;
    unsigned char   reserved_2D;
    unsigned char   order_loop;
    unsigned char   reserved_2F;
    unsigned char   volume;
    unsigned char   speed;
    unsigned char   reserved_32[12];
    unsigned short  patseg;
  };

  struct rat_event
  {
    unsigned char   note;
    unsigned char   instrument;
    unsigned char   volume;
    unsigned char   fx;
    unsigned char   fxp;
  };

  struct rat_instrument
  {
    unsigned short  freq;
    unsigned char   reserved_2[2];
    unsigned char   mod_ctrl;
    unsigned char   car_ctrl;
    unsigned char   mod_volume;   
    unsigned char   car_volume;   
    unsigned char   mod_AD;       
    unsigned char   car_AD;       
    unsigned char   mod_SR;       
    unsigned char   car_SR;       
    unsigned char   mod_wave;
    unsigned char   car_wave;
    unsigned char   connect;
    unsigned char   reserved_F;
    unsigned char   volume;
    unsigned char   reserved_11[3];
  };

  struct
  {
    rat_header      hdr;

    unsigned char   volume;
    unsigned char   order_pos;
    unsigned char   pattern_pos;

    unsigned char   *order;

    rat_instrument  *inst;

    rat_event       tracks[256][64][9];

    struct
    {
      unsigned char   instrument;
      unsigned char   volume;
      unsigned char   fx;
      unsigned char   fxp;
    } channel[9];
  } rat;
  //
  bool            xadplayer_load(istream &f);
  void            xadplayer_rewind(unsigned int subsong);
  void            xadplayer_update();
  float           xadplayer_getrefresh();
  std::string	    xadplayer_gettype();
  std::string     xadplayer_gettitle();
  unsigned int    xadplayer_getinstruments();
  //
private:
  unsigned char   __rat_calc_volume(unsigned char ivol, unsigned char cvol, unsigned char gvol);
};

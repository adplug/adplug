/*
  [xad] HYBRID player, by Riven the Mage <riven@ok.ru>
*/

#include "xad.h"

class CxadhybridPlayer: public CxadPlayer
{
public:
  CxadhybridPlayer(Copl *newopl): CxadPlayer(newopl)
    { }

protected:
  struct hyb_instrument
  {
    char            name[7];
    unsigned char   mod_wave;
    unsigned char   mod_AD;
    unsigned char   mod_SR;
    unsigned char   mod_crtl;
    unsigned char   mod_volume;
    unsigned char   car_wave;
    unsigned char   car_AD;
    unsigned char   car_SR;
    unsigned char   car_crtl;
    unsigned char   car_volume;
    unsigned char   connect;
  };

  struct
  {
    unsigned char   order_pos;
    unsigned char   pattern_pos;

    unsigned char   *order;

    hyb_instrument  *inst;

    struct
    {
      unsigned short  freq;
      unsigned short  freq_slide;
    } channel[9];

    unsigned char   speed;
    unsigned char   speed_counter;
  } hyb;
  //
  bool            xadplayer_load(istream &f);
  void            xadplayer_rewind(unsigned int subsong);
  void            xadplayer_update();
  float           xadplayer_getrefresh();
  std::string     xadplayer_gettype();
  std::string     xadplayer_getinstrument(unsigned int i);
  unsigned int    xadplayer_getinstruments();
};

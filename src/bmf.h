/*
  [xad] BMF player, by Riven the Mage <riven@ok.ru>
*/

#include "xad.h"

class CxadbmfPlayer: public CxadPlayer
{
public:
  CxadbmfPlayer(Copl *newopl): CxadPlayer(newopl)
    { };
  ~CxadbmfPlayer()
    { };

protected:
  enum { BMF0_9B, BMF1_1, BMF1_2 };
  //
  struct bmf_event
  {
    unsigned char   note;
    unsigned char   delay;
    unsigned char   volume;
    unsigned char   instrument;
    unsigned char   cmd;
    unsigned char   cmd_data;
  };

  struct
  {
    unsigned char   version;
    char            title[36];
    char            author[36];
    float           timer;
    unsigned char   speed;
  
    struct
    {
      char            name[11];
      unsigned char   data[13];
    } instruments[32];

    bmf_event       streams[9][1024];

    int             active_streams;

    struct
    {
      unsigned short  stream_position;
      unsigned char   delay;
      unsigned short  loop_position;
      unsigned char   loop_counter;
    } channel[9];
  } bmf;
  //
  bool            xadplayer_load(istream &f);
  void            xadplayer_rewind(unsigned int subsong);
  void            xadplayer_update();
  float           xadplayer_getrefresh();
  std::string     xadplayer_gettype();
  std::string     xadplayer_gettitle();
  std::string     xadplayer_getauthor();
  std::string     xadplayer_getinstrument(unsigned int i);
  unsigned int    xadplayer_getinstruments();
  //
private:
  int             __bmf_convert_stream(unsigned char *stream, int channel);
};

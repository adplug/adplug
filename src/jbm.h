/*
 * jbm.h - JBM Player by Dennis Lindroos <lindroos@nls.fi>
 */

#ifndef H_JBM
#define H_JBM

#include "player.h"

class CjbmPlayer: public CPlayer
{
 public:
  static CPlayer *factory(Copl *newopl);

  CjbmPlayer(Copl *newopl) : CPlayer(newopl), m(0)
    { };
  ~CjbmPlayer()
    { if(m != NULL) delete [] m; };

  bool load(const std::string &filename, const CFileProvider &fp);
  bool update();
  void rewind(int subsong);

  float getrefresh() { return timer; };

  std::string gettype() {
    return std::string(flags&1 ? "JBM Adlib Music [rhythm mode]" :
                                 "JBM Adlib Music");
  };
  // std::string gettitle() { return std::string(); };
  std::string getauthor() { return std::string("Johannes Bjerregaard"); };

  // unsigned int getsubsongs() { return 1; };

 protected:

  unsigned char *m;
  float timer;
  unsigned short flags, voicemask;
  unsigned short seqtable, seqcount;
  unsigned short instable, inscount;
  unsigned short *sequences;
  unsigned char bdreg; 

  typedef struct {
	unsigned short trkpos, trkstart, seqpos;
	unsigned char seqno, note;
	short vol;
	short delay;
	short instr;
	unsigned char frq[2];
	unsigned char ivol, dummy;
  } JBMVoice;

  JBMVoice voice[11];
    
 private:
  //void calc_opl_frequency(JBMVoice *);
  void set_opl_instrument(int, JBMVoice *); 
  void opl_noteonoff(int, JBMVoice *, bool);
};

#endif

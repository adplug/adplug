void adlibinit(long dasamplerate,long danumspeakers,long dabytespersample);
void adlib0(long i,long v);
void adlibgetsample(void *sndptr,long numbytes);
void adlibsetvolume(int i);
void randoinsts();
extern float lvol[9],rvol[9],lplc[9],rplc[9];

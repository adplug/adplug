#ifndef __ADLIBEMU_H__
#define __ADLIBEMU_H__

#define MAXCELLS 18
#define FIFOSIZ 256

typedef void (*ADLIB_UPDATEHANDLER)(int param,int min_interval_us);

typedef struct
{
	float val, t, tinc, vol, sustain, amp, mfb;
	float a0, a1, a2, a3, decaymul, releasemul;
	short *waveform;
	long wavemask;
	void (*cellfunc)(void *, float);
	unsigned char flags, dum0, dum1, dum2;
} celltype;

typedef struct
{
	long numspeakers, bytespersample;
	float recipsamp;
	float nfrqmul[16];
	celltype cell[MAXCELLS];
	unsigned char reg[256];

	float lvol[9];  //Volume multiplier on left speaker
	float rvol[9];  //Volume multiplier on right speaker
	long lplc[9];   //Samples to delay on left speaker
	long rplc[9];   //Samples to delay on right speaker

	long nlvol[9];
	long nrvol[9];
	long nlplc[9];
	long nrplc[9];
	long rend;

	float *rptr[9];
	float *nrptr[9];
	float rbuf[9][FIFOSIZ*2];
	float snd[FIFOSIZ*2];

	ADLIB_UPDATEHANDLER UpdateHandler;
	int UpdateParam;

} ADLIB_STATE;

ADLIB_STATE *adlibinit(long dasamplerate, long danumspeakers, long dabytespersample);
void adlibwrite(ADLIB_STATE *adlib, long i, long v);
void adlibgetsample(ADLIB_STATE *adlib, void *sndptr, long numsamples);
void adlibreset(ADLIB_STATE *adlib);
void adlibshutdown(ADLIB_STATE *adlib);
void adlibsetupdatehandler(ADLIB_STATE *adlib, ADLIB_UPDATEHANDLER UpdateHandler, int param);

#endif

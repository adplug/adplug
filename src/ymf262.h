#ifndef YMF262_H
#define YMF262_H

#define INLINE inline
#define HAS_YMF262 1

namespace YMF262{


#define BUILD_YMF262 (HAS_YMF262)


/* select number of output bits: 8 or 16 */
#define OPL3_SAMPLE_BITS 16

/* compiler dependence */
#ifndef OSD_CPU_H
#define OSD_CPU_H
typedef unsigned char	UINT8;   /* unsigned  8bit */
typedef unsigned short	UINT16;  /* unsigned 16bit */
typedef unsigned int	UINT32;  /* unsigned 32bit */
typedef signed char		INT8;    /* signed  8bit   */
typedef signed short	INT16;   /* signed 16bit   */
typedef signed int		INT32;   /* signed 32bit   */
#endif

#if (OPL3_SAMPLE_BITS==16)
typedef INT16 OPL3SAMPLE;
#endif
#if (OPL3_SAMPLE_BITS==8)
typedef INT8 OPL3SAMPLE;
#endif





#if BUILD_YMF262


	typedef void (*OPL3_TIMERHANDLER)(int channel,double interval_Sec);
	typedef void (*OPL3_IRQHANDLER)(int param,int irq);
	typedef void (*OPL3_UPDATEHANDLER)(int param,int min_interval_us);


typedef struct{
	UINT32	ar;			/* attack rate: AR<<2			*/
	UINT32	dr;			/* decay rate:  DR<<2			*/
	UINT32	rr;			/* release rate:RR<<2			*/
	UINT8	KSR;		/* key scale rate				*/
	UINT8	ksl;		/* keyscale level				*/
	UINT8	ksr;		/* key scale rate: kcode>>KSR	*/
	UINT8	mul;		/* multiple: mul_tab[ML]		*/

	/* Phase Generator */
	UINT32	Cnt;		/* frequency counter			*/
	UINT32	Incr;		/* frequency counter step		*/
	UINT8   FB;			/* feedback shift value			*/
	INT32   *connect;	/* slot output pointer			*/
	INT32   op1_out[2];	/* slot1 output for feedback	*/
	UINT8   CON;		/* connection (algorithm) type	*/

	/* Envelope Generator */
	UINT8	eg_type;	/* percussive/non-percussive mode */
	UINT8	state;		/* phase type					*/
	UINT32	TL;			/* total level: TL << 2			*/
	INT32	TLL;		/* adjusted now TL				*/
	INT32	volume;		/* envelope counter				*/
	UINT32	sl;			/* sustain level: sl_tab[SL]	*/

	UINT32	eg_m_ar;	/* (attack state)				*/
	UINT8	eg_sh_ar;	/* (attack state)				*/
	UINT8	eg_sel_ar;	/* (attack state)				*/
	UINT32	eg_m_dr;	/* (decay state)				*/
	UINT8	eg_sh_dr;	/* (decay state)				*/
	UINT8	eg_sel_dr;	/* (decay state)				*/
	UINT32	eg_m_rr;	/* (release state)				*/
	UINT8	eg_sh_rr;	/* (release state)				*/
	UINT8	eg_sel_rr;	/* (release state)				*/

	UINT32	key;		/* 0 = KEY OFF, >0 = KEY ON		*/

	/* LFO */
	UINT32	AMmask;		/* LFO Amplitude Modulation enable mask */
	UINT8	vib;		/* LFO Phase Modulation enable flag (active high)*/

	/* waveform select */
	UINT8	waveform_number;
	unsigned int wavetable;

//unsigned char reserved[128-84];//speedup: pump up the struct size to power of 2
unsigned char reserved[128-100];//speedup: pump up the struct size to power of 2

} OPL3_SLOT;

typedef struct{
	OPL3_SLOT SLOT[2];

	UINT32	block_fnum;	/* block+fnum					*/
	UINT32	fc;			/* Freq. Increment base			*/
	UINT32	ksl_base;	/* KeyScaleLevel Base step		*/
	UINT8	kcode;		/* key code (for key scaling)	*/

	/*
	   there are 12 2-operator channels which can be combined in pairs
	   to form six 4-operator channel, they are:
		0 and 3,
		1 and 4,
		2 and 5,
		9 and 12,
		10 and 13,
		11 and 14
	*/
	UINT8	extended;	/* set to 1 if this channel forms up a 4op channel with another channel(only used by first of pair of channels, ie 0,1,2 and 9,10,11) */

unsigned char reserved[512-272];//speedup:pump up the struct size to power of 2

} OPL3_CH;

/* OPL3 state */
typedef struct {
	OPL3_CH	P_CH[18];				/* OPL3 chips have 18 channels	*/

	UINT32	pan[18*4];				/* channels output masks (0xffffffff = enable); 4 masks per one channel */
	UINT32	pan_ctrl_value[18];		/* output control values 1 per one channel (1 value contains 4 masks) */

	UINT32	eg_cnt;					/* global envelope generator counter	*/
	UINT32	eg_timer;				/* global envelope generator counter works at frequency = chipclock/288 (288=8*36) */
	UINT32	eg_timer_add;			/* step of eg_timer						*/
	UINT32	eg_timer_overflow;		/* envelope generator timer overlfows every 1 sample (on real chip) */

	UINT32	fn_tab[1024];			/* fnumber->increment counter	*/

	/* LFO */
	UINT8	lfo_am_depth;
	UINT8	lfo_pm_depth_range;
	UINT32	lfo_am_cnt;
	UINT32	lfo_am_inc;
	UINT32	lfo_pm_cnt;
	UINT32	lfo_pm_inc;

	UINT32	noise_rng;				/* 23 bit noise shift register	*/
	UINT32	noise_p;				/* current noise 'phase'		*/
	UINT32	noise_f;				/* current noise period			*/

	UINT8	OPL3_mode;				/* OPL3 extension enable flag	*/

	UINT8	rhythm;					/* Rhythm mode					*/

	int		T[2];					/* timer counters				*/
	int		TC[2];
	UINT8	st[2];					/* timer enable					*/

	UINT32	address;				/* address register				*/
	UINT8	status;					/* status flag					*/
	UINT8	statusmask;				/* status mask					*/

	UINT8	nts;					/* NTS (note select)			*/

	/* external event callback handlers */
	OPL3_TIMERHANDLER  TimerHandler;/* TIMER handler				*/
	int TimerParam;					/* TIMER parameter				*/
	OPL3_IRQHANDLER    IRQHandler;	/* IRQ handler					*/
	int IRQParam;					/* IRQ parameter				*/
	OPL3_UPDATEHANDLER UpdateHandler;/* stream update handler		*/
	int UpdateParam;				/* stream update parameter		*/

	UINT8 type;						/* chip type					*/
	int clock;						/* master clock  (Hz)			*/
	int rate;						/* sampling rate (Hz)			*/
	double freqbase;				/* frequency base				*/
	double TimerBase;				/* Timer base time (==sampling time)*/
} OPL3;



	class Class{
	public:
		Class();

		///////
		#define TL_RES_LEN		(256)	/* 8 bits addressing (real chip) */

		/* sinwave entries */
		#define SIN_BITS		10
		#define SIN_LEN			(1<<SIN_BITS)
		#define SIN_MASK		(SIN_LEN-1)


		/*	TL_TAB_LEN is calculated as:

		*	(12+1)=13 - sinus amplitude bits     (Y axis)
		*   additional 1: to compensate for calculations of negative part of waveform
		*   (if we don't add it then the greatest possible _negative_ value would be -2
		*   and we really need -1 for waveform #7)
		*	2  - sinus sign bit           (Y axis)
		*	TL_RES_LEN - sinus resolution (X axis)
		*/

		#define TL_TAB_LEN (13*2*TL_RES_LEN)
		signed int tl_tab[TL_TAB_LEN];

		#define ENV_QUIET		(TL_TAB_LEN>>4)

		/* sin waveform table in 'decibel' scale */
		/* there are eight waveforms on OPL3 chips */
		unsigned int sin_tab[SIN_LEN * 8];


		int num_lock;

		/* work table */
		void *cur_chip;			/* current chip point */

		OPL3_SLOT *SLOT7_1,*SLOT7_2,*SLOT8_1,*SLOT8_2;

		signed int phase_modulation;		/* phase modulation input (SLOT 2) */
		signed int phase_modulation2;	/* phase modulation input (SLOT 3 in 4 operator channels) */
		signed int chanout[18];			/* 18 channels */


		UINT32	LFO_AM;
		INT32	LFO_PM;

		int YMF262NumChips;				/* number of chips */
		////////
		

		///////
		INLINE void advance(OPL3 *chip);
		INLINE void advance_lfo(OPL3 *chip);
		INLINE void chan_calc( OPL3_CH *CH );
		INLINE void chan_calc_ext( OPL3_CH *CH );
		INLINE void chan_calc_rhythm( OPL3_CH *CH, unsigned int noise );
		void OPL3WriteReg(OPL3 *chip, int r, int v);
		int init_tables(void);
		INLINE signed int op_calc(UINT32 phase, unsigned int env, signed int pm, unsigned int wave_tab);
		INLINE signed int op_calc1(UINT32 phase, unsigned int env, signed int pm, unsigned int wave_tab);
		int OPL3_LockTable(void);
		void OPL3_UnLockTable(void);
		void OPL3ResetChip(OPL3 *chip);
		OPL3 *OPL3Create(int type, int clock, int rate);
		void OPL3Destroy(OPL3 *chip);
		int OPL3Write(OPL3 *chip, int a, int v);
		//////


		int  YMF262Init(int num, int clock, int rate);
		void YMF262Shutdown(void);
		void YMF262ResetChip(int which);
		int  YMF262Write(int which, int a, int v);
		unsigned char YMF262Read(int which, int a);
		int  YMF262TimerOver(int which, int c);
		void YMF262UpdateOne(int which, INT16 *buffer, int length);

		void YMF262SetTimerHandler(int which, OPL3_TIMERHANDLER TimerHandler, int channelOffset);
		void YMF262SetIRQHandler(int which, OPL3_IRQHANDLER IRQHandler, int param);
		void YMF262SetUpdateHandler(int which, OPL3_UPDATEHANDLER UpdateHandler, int param);
	};
#endif

} //end namespace YMF262

#endif /* YMF262_H */

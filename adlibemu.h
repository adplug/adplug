/*
 * adlibemu.h - Header file for Ken Silverman's OPL2 emulator, by Simon Peter (dn.tlp@gmx.net)
 */

void adlibinit (long dasamplerate, long danumspeakers, long dabytespersample);
/*
 * Initialize OPL2 emulator.
 *
 * dasamplerate		= Output sample rate
 * danumspeakers	= Mono or Stereo
 * dabytespersample	= 8 or 16 bit
 */

void adlib0 (long i, long v);
/*
 * Output to OPL2.
 *
 * i	= Index register
 * v	= Output register
 */

void adlibgetsample (void *sndptr, long numbytes);
/*
 * Get samples to sample buffer.
 *
 * sndptr	= pointer to sample buffer
 * numbytes	= number of bytes to write
 */

#ifndef __TIMER_H
#define __TIMER_H

int  tmInit(void (*)(), int, int);
void tmClose();
void tmSetNewRate(int);
int  tmGetTicker();
void tmSetTicker(long t);
int  tmGetTimer();
int  tmGetCpuUsage();
void tmSetSecure();
void tmReleaseSecure();

#endif

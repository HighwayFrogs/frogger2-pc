#include "bbtimer.h"

#ifndef PSX_VERSION
#define GetRCnt(x) (0)
#define ResetRCnt(x) (0)
#endif

// 
// void ZeroTime(void)
// {
// 	ResetRCnt(1);
// }
// 
// 
// void TimerStart(BBTIMER* t)
// {
// 	t->start = GetRCnt(1);
// 	t->on = 1;
// }
// 
// void TimerRestart(BBTIMER* t)
// {
// 	t->start = GetRCnt(1);
// 	t->total = 0;
// 	t->on = 1;
// }
// 
// void TimerStop(BBTIMER* t)
// {
// 	if(t->on)
// 	{
// 		t->total = GetRCnt(1) - t->start;
// 		t->on = 0;
// 	}
// }
// 
// void TimerStopAdd(BBTIMER* t)
// {
// 	if(t->on)
// 	{
// 		t->total += GetRCnt(1) - t->start;
// 		t->on = 0;
// 	}
// }

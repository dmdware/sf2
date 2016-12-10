













#ifndef SIMFLOW_H
#define SIMFLOW_H

#include "../platform.h"

extern unsigned __int64 g_simframe;
extern unsigned __int64 g_cmdframe;	//net frames follow sim frames except when there's a pause, simframes will stop but cmdframes will continue

#define SPEED_PAUSE		1
#define SPEED_PLAY		2
#define SPEED_FAST		3

extern unsigned char g_speed;
extern ecbool g_gameover;

void UpdSpeed();

#endif
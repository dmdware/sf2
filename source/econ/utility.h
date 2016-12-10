












#ifndef UTILITY_H
#define UTILITY_H

#include "../platform.h"

//#define MAX_UTIL	1000000000	//possible overflow?
//make sure this value is bigger than any combined obtainable revenue of building from labourer spending
//edit: and multiplied by distance?
//#define MAX_UTIL	200000000	//200mill
#define MAX_UTIL	2000000000	//2000mill
//#define MAX_UTIL	2000000
//#define MAX_UTIL	((__int64)1<<40)

int PhUtil(int price, int cmdist);
int GlUtil(int price);
int InvPhUtilD(int util, int price);
int InvPhUtilP(int util, int cmdist);
int InvGlUtilP(int util);
int JobUtil(int wage, int cmdist, int workdelay);
int InvJobUtilP(int util, int cmdist, int workdelay);

//every 1 m of walking = $3
#if 0
#define DIST_EFFECT	1
#define WAGE_EFFECT	(1/3000000)
#define INV_WAGE_EFFECT	3000000
#else
//every 1 cm of walking = $0.3
#define DIST_EFFECT	(1/1)
#define INV_DIST_EFFECT	1
#define WAGE_EFFECT	(1/3)
#define INV_WAGE_EFFECT	3
#endif

#endif

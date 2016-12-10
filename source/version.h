













#ifndef VERSION_H
#define VERSION_H

#include "platform.h"

#define VERSTAT_PA	0	//pre-alpha
#define VERSTAT_A	1	//alpha
#define VERSTAT_B	2	//beta
#define VERSTAT_RC	3	//release candidate
#define VERSTAT_F	4	//final

extern const char* VERSTAT_STR[5];

unsigned int MakeVer(unsigned char major, unsigned char featureset, unsigned char attempt, unsigned char status, unsigned char postrelease);
void ParseVer(unsigned int number, unsigned char* major, unsigned char* featureset, unsigned char* attempt, unsigned char* status);
void VerStr(unsigned int number, char* str);

#endif
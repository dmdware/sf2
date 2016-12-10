













#include "version.h"
#include "platform.h"

#define VERSTAT_PA	0	//pre-alpha
#define VERSTAT_A	1	//alpha
#define VERSTAT_B	2	//beta
#define VERSTAT_RC	3	//release candidate
#define VERSTAT_F	4	//final

const char* VERSTAT_STR[5] = { "pa", "a", "b", "rc", "f" };

unsigned int MakeVer(unsigned char major, unsigned char featureset, unsigned char attempt, unsigned char status, unsigned char postrelease)
{
	unsigned int number = 0;
	number = number | ( (major & (unsigned int)7) << (32 - 3) );
	number = number | ( (status & (unsigned int)7) << (32 - 6) );
	number = number | ( (postrelease & (unsigned int)255) << (32 - 16) );
	number = number | ( (featureset & (unsigned int)255) << (32 - 24) );
	number = number | ( (attempt & (unsigned int)255) << (32 - 32) );
	return number;
}

void ParseVer(unsigned int number, unsigned char* major, unsigned char* featureset, unsigned char* attempt, unsigned char* status, unsigned char* postrelease)
{
	*major = (unsigned char)(number >> (32 - 3)) & (unsigned int)7;			//bits [0..2], max value 7
	*status = (unsigned char)(number >> (32 - 6)) & (unsigned int)7;			//bits [3..5], max value 7
	*postrelease = (unsigned char)(number >> (32 - 16)) & (unsigned int)255;	//bits [8..15], max value 255
	*featureset = (unsigned char)(number >> (32 - 24)) & (unsigned int)255;		//bits [16..23], max value 255
	*attempt = (unsigned char)(number >> (32 - 32)) & (unsigned int)255;		//bits [24..31], max value 255
	//bits reserved: [6,7]
}

void VerStr(unsigned int number, char* str)
{
#if 0
	unsigned char major;
	unsigned char featureset;
	unsigned char attempt;
	unsigned char status;
	unsigned char postrelease;
	ParseVer(number, &major, &featureset, &attempt, &status, &postrelease);
	sprintf(str, "%u.%u.%u%s%u", (unsigned int)major, (unsigned int)featureset, (unsigned int)attempt, VERSTAT_STR[status], (unsigned int)postrelease);
#else
	sprintf(str, "%u", number);
#endif
}
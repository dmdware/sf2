












#include "../platform.h"
#include "checksum.h"
#include "../utils.h"

//TO DO: mv as circles collision

unsigned int AddCheck(unsigned int sum, unsigned int next)
{
	sum++;
	sum = sum ^ next;
	//sum = (sum * 2) + (sum >> 7);	//for unsigned char
	sum = (sum * 2) + (sum >> 31);	//for unsigned int
	//sum = (sum * 2) + (sum >> (sizeof(sum) * 8 - 1));	//automatically determine
	return sum;
}

unsigned int CheckSum(const char* fullpath)
{
	unsigned int checksum = 0;
	FILE* fp = fopen(fullpath, "rb");

	//Log("start check %d %u", i, checksum);

	if(!fp)
		return checksum;

	int i = 0;
	int len = 0;

	fseek(fp, 0L, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	//wrong: will read an extra byte on mac/ios!
	//while(!feof(fp))
	//edit: nvm problem with copying but same issue
	for(i=0; i<len; i++)
	{
		unsigned char next;
		fread(&next, sizeof(unsigned char), 1, fp);
		checksum = AddCheck(checksum, next);
		
		//++i;
		//Log("check %d %u", i, checksum);
	}

	fclose(fp);

	//Log("end check %d %u\r\n", i, checksum);

#if 0
	char msg[128];
	sprintf(msg, "check %u", checksum);
	InfoMess("c", msg);
#endif

	return checksum;
}
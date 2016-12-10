












#include "../platform.h"
#include "random.h"

// http://stackoverflow.com/questions/4768180/rand-implementation

static unsigned __int64 next = 1;

unsigned int rand2() // RAND_MAX assumed to be 32767
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}

void srand2(unsigned int seed)
{
    next = seed;
}

// own implementation

// http://en.wikipedia.org/wiki/Circular_shift
/*
 * Shift operations in C are only defined for shift values which are
 * not negative and smaller than sizeof(value) * CHAR_BIT.
 */
 
unsigned int rotl(unsigned int value, int shift) {
    return (value << shift) | (value >> (sizeof(value) * CHAR_BIT - shift));
}
 
unsigned int rotr(unsigned int value, int shift) {
    return (value >> shift) | (value << (sizeof(value) * CHAR_BIT - shift));
}

static unsigned int counter = 0;

void srand3(unsigned int seed)
{
	//srand(seed);
	counter = seed;
	next = seed;
	//settable(12345,65435,34221,12345,9983651,95746118);
	//settable(12345,65435,seed,12345,9983651,95746118);
	//settable(12345,65435,34221,seed,9983651,95746118);
}

unsigned int rand3()
{
    //next = next * 1103515245 + 12345;
	//for(unsigned char i=0; i<=next%5; i++)
	//	next = next ^ (next << (next % 16) + next >> (next % 16)) + 1;
	//next = (next << (next % 16) + next >> (next % 16));
	
	//return next;

	//const unsigned int mask = 0xffffffff;

	//little endian?
	//const unsigned int leftmask = 0xffff0000;
	//const unsigned int rightmask = 0x0000ffff;
	
	//big endian?
	//const unsigned int leftmask = 0x0000ffff;
	//const unsigned int rightmask = 0xffff0000;

	//need masks to get over undefined behaviour on certain systems of shift overflow

	//next = next ^ 3;
	//unsigned int shiftamount = ((unsigned int)32 - next) % 32;
	//unsigned int leftshift = (next & rightmask) << 16;
	//unsigned int rightshift = (next & leftmask) >> 16;
	//next = next ^ ( leftshift | rightshift );

	//unsigned int original = next;
	//next = next ^ 0x0f0f0f0f;
	//next = next + 1;
    //unsigned int temp = next * 1103515245 + 12345;
    //temp = (unsigned int)(temp/65536) % 32768;
	//next = next ^ ~temp;
	//next = rotr(next, next % 32);

	//unsigned int temp = next % 32;
	//next = next * 2 + 1;
	//next = next ^ 0x4a3b2c1d;
	//next = rotl(next, 1) ^ counter;
	//counter++;
	//return next;
	
	//just keep a counter and rearrange its bits to give a non-linear result
	//basically a sequence with a period of 2^32
	//to get a different sequence, change which bits are switched

#if 0

	const unsigned int bit00mask = 1 << 0;
	const unsigned int bit01mask = 1 * 2;
	const unsigned int bit02mask = 1 << 2;
	const unsigned int bit03mask = 1 << 3;
	const unsigned int bit04mask = 1 << 4;
	const unsigned int bit05mask = 1 << 5;
	const unsigned int bit06mask = 1 << 6;
	const unsigned int bit07mask = 1 << 7;
	const unsigned int bit08mask = 1 << 8;
	const unsigned int bit09mask = 1 << 9;
	const unsigned int bit10mask = 1 << 10;
	const unsigned int bit11mask = 1 << 11;
	const unsigned int bit12mask = 1 << 12;
	const unsigned int bit13mask = 1 << 13;
	const unsigned int bit14mask = 1 << 14;
	const unsigned int bit15mask = 1 << 15;
	const unsigned int bit16mask = 1 << 16;
	const unsigned int bit17mask = 1 << 17;
	const unsigned int bit18mask = 1 << 18;
	const unsigned int bit19mask = 1 << 19;
	const unsigned int bit20mask = 1 << 20;
	const unsigned int bit21mask = 1 << 21;
	const unsigned int bit22mask = 1 << 22;
	const unsigned int bit23mask = 1 << 23;
	const unsigned int bit24mask = 1 << 24;
	const unsigned int bit25mask = 1 << 25;
	const unsigned int bit26mask = 1 << 26;
	const unsigned int bit27mask = 1 << 27;
	const unsigned int bit28mask = 1 << 28;
	const unsigned int bit29mask = 1 << 29;
	const unsigned int bit30mask = 1 << 30;
	const unsigned int bit31mask = 1 << 31;
	
	unsigned int oldbit00 = ( counter & bit00mask ) >> 0;
	unsigned int oldbit01 = ( counter & bit01mask ) >> 0;
	unsigned int oldbit02 = ( counter & bit02mask ) >> 0;
	unsigned int oldbit03 = ( counter & bit03mask ) >> 0;
	unsigned int oldbit04 = ( counter & bit04mask ) >> 0;
	unsigned int oldbit05 = ( counter & bit05mask ) >> 0;
	unsigned int oldbit06 = ( counter & bit06mask ) >> 0;
	unsigned int oldbit07 = ( counter & bit07mask ) >> 0;
	unsigned int oldbit08 = ( counter & bit08mask ) >> 0;
	unsigned int oldbit09 = ( counter & bit09mask ) >> 0;
	unsigned int oldbit10 = ( counter & bit10mask ) >> 0;
	unsigned int oldbit11 = ( counter & bit11mask ) >> 0;
	unsigned int oldbit12 = ( counter & bit12mask ) >> 0;
	unsigned int oldbit13 = ( counter & bit13mask ) >> 0;
	unsigned int oldbit14 = ( counter & bit14mask ) >> 0;
	unsigned int oldbit15 = ( counter & bit15mask ) >> 0;
	unsigned int oldbit16 = ( counter & bit16mask ) >> 0;
	unsigned int oldbit17 = ( counter & bit17mask ) >> 0;
	unsigned int oldbit18 = ( counter & bit18mask ) >> 0;
	unsigned int oldbit19 = ( counter & bit19mask ) >> 0;
	unsigned int oldbit20 = ( counter & bit20mask ) >> 0;
	unsigned int oldbit21 = ( counter & bit21mask ) >> 0;
	unsigned int oldbit22 = ( counter & bit22mask ) >> 0;
	unsigned int oldbit23 = ( counter & bit23mask ) >> 0;
	unsigned int oldbit24 = ( counter & bit24mask ) >> 0;
	unsigned int oldbit25 = ( counter & bit25mask ) >> 0;
	unsigned int oldbit26 = ( counter & bit26mask ) >> 0;
	unsigned int oldbit27 = ( counter & bit27mask ) >> 0;
	unsigned int oldbit28 = ( counter & bit28mask ) >> 0;
	unsigned int oldbit29 = ( counter & bit29mask ) >> 0;
	unsigned int oldbit30 = ( counter & bit30mask ) >> 0;
	unsigned int oldbit31 = ( counter & bit31mask ) >> 0;

	counter++;
	
	unsigned int newbit00 = oldbit00 << 0;
	unsigned int newbit01 = oldbit00 * 2;
	unsigned int newbit02 = oldbit00 << 2;
	unsigned int newbit03 = oldbit00 << 3;
	unsigned int newbit04 = oldbit00 << 4;
	unsigned int newbit05 = oldbit00 << 5;
	unsigned int newbit06 = oldbit00 << 6;
	unsigned int newbit07 = oldbit00 << 7;
	unsigned int newbit08 = oldbit00 << 8;
	unsigned int newbit09 = oldbit00 << 9;
	unsigned int newbit10 = oldbit00 << 10;
	unsigned int newbit11 = oldbit00 << 11;
	unsigned int newbit12 = oldbit00 << 12;
	unsigned int newbit13 = oldbit00 << 13;
	unsigned int newbit14 = oldbit00 << 14;
	unsigned int newbit15 = oldbit00 << 15;
	unsigned int newbit16 = oldbit00 << 16;
	unsigned int newbit17 = oldbit00 << 17;
	unsigned int newbit18 = oldbit00 << 18;
	unsigned int newbit19 = oldbit00 << 19;
	unsigned int newbit20 = oldbit00 << 20;
	unsigned int newbit21 = oldbit00 << 21;
	unsigned int newbit22 = oldbit00 << 22;
	unsigned int newbit23 = oldbit00 << 23;
	unsigned int newbit24 = oldbit00 << 24;
	unsigned int newbit25 = oldbit00 << 25;
	unsigned int newbit26 = oldbit00 << 26;
	unsigned int newbit27 = oldbit00 << 27;
	unsigned int newbit28 = oldbit00 << 28;
	unsigned int newbit29 = oldbit00 << 29;
	unsigned int newbit30 = oldbit00 << 30;
	unsigned int newbit31 = oldbit00 << 31;

	unsigned int r = 0;

	r = r | newbit00;

#elif 0

	//using loops

	ecbool old[32];
	
	for(unsigned char i=0; i<32; i++)
		old[i] = (ecbool)(counter & (1 << i));

	unsigned int r = 0;

	for(unsigned char b1=0; b1<32; b1++)
	{
		unsigned char b2;
		//rotate left by 16+1 bits
		b2 = (16+1 + b1) % 32;
		//switch every two bits
		unsigned char twopair = b1 / 2;
		unsigned char twomember = b1 % 2;
		b2 = twopair * 2 + (1 - twomember);
		//switch every four bits (outer bits of the four pair)
		unsigned char fourpair = b1 / 4;
		unsigned char fourmember = b1 % 4;
		b2 = fourpair * 4 + (3 - fourmember);

		//don't shuffle bits 0 and 15
		//so that we get odd/even counter
		if(b1 == 0 || b2 == 0)
			b2 = b1;

		unsigned int bit = (unsigned int)old[b1] << b2;
		r = r | bit;
	}

	counter++;

	return r;

#elif 1

	unsigned int r = counter ^ 0x4a3b2c1d;
	//counter++;
	r = rotl(r, counter % 32);
	//counter++;
	r = r ^ counter;
	counter++;
	//r = r + (0xffff0000 - counter);
	return r;
#elif 0
	//unsigned int next = counter;
	//counter++;
    //next = next * 1103515245 + 12345;
    //next = (unsigned int)(next/65536) % 32768;
	//next = rotl(next, counter % 32);
	//return next;

    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
#endif

#if 0
	unsigned char times = next % 33 + 1;

	for(unsigned char i=1; i<=times; i++)
	{
		unsigned int rot = rotl(next, (next+i) % 32);
		next = next + 2;
		next = next ^ ~rot;
	}
#elif 0
	unsigned int rot = rotl(next, (next+1) % 32);
	//next = next + 2;
	unsigned int temp = original ^ ~rot;
	next = next ^ (temp ^ rand2());
	
#if 0
	rot = rotl(next, (original+1) % 32);
	next = temp / 65536;
	next = next ^ ~rot;
	
	rot = rotl(next, (temp+1) % 32);
	temp = next + 1;
	next = temp ^ ~rot;
#endif
#endif


	//if you find a repeat, add here
	//const unsigned int repeat1 = 4294967233;
	//next = original + repeat1;

	//return next;
	//counter++;
	//return CONG + counter;
	//return rand();
}

// http://en.wikipedia.org/wiki/Randonumber_generation

static unsigned int w = 1;    /* must not be zero, nor 0x464fffff */
static unsigned int z = 1;    /* must not be zero, nor 0x9068ffff */

void srand4(unsigned int s1, unsigned int s2)
{
	w = s1;
	z = s2;
}
 
unsigned int rand4()
{
    z = 36969 * (z & 65535) + (z >> 16);
    w = 18000 * (w & 65535) + (w >> 16);
    return (z << 16) + w;  /* 32-bit result */
}

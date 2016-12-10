

#include "../utils.h"
#include "localmap.h"

		HASHINT m[HASHLEVELS];

void TestHash()
{
	//m = new HASHINT [HASHLEVELS];
	//m2 = new HASHINT [HASHLEVELS];
	//	memset(m, 0, HASHLEVELS * sizeof(HASHINT));
	//	memset(m2, 0, HASHLEVELS * sizeof(HASHINT));

}

void pb(char* c, HASHINT i)
{
	char mc[HASHBITS+1];
	mc[HASHBITS]=0;

	for(int in=0; in<HASHBITS; ++in, i>>=1)
		//strcat(c, (i%2==0)?"0":"1");
		mc[HASHBITS-in-1] = ((i%2==0)?'0':'1');

	strcat(c,mc);
}

void pb22(char* c, HASHINT i)
{
	char mc[HASHBITS*2+1];
	mc[HASHBITS*2]=0;

	for(int in=0; in<HASHBITS*2; ++in, i>>=1)
		//strcat(c, (i%2==0)?"0":"1");
		mc[HASHBITS*2-in-1] = ((i%2==0)?'0':'1');

	strcat(c,mc);
}

void pb2(char* c, HASHINT t, HASHINT m)
{
	char mc[HASHBITS+1];
	mc[HASHBITS]=0;

	for(int in=0; in<HASHBITS; ++in, t>>=1, m>>=1)
		//strcat(c, (i%2==0)?"0":"1");
		mc[HASHBITS-in-1] = (m%2==1)?('m') : ((t%2==0)?'0':'1');

	strcat(c,mc);
}

HASHINT Hash(HASHINT* mval, HASHINT oval, HASHADDR level, ecbool echo)
{
	if(level >= HASHLEVELS)
		return oval;

#ifdef HASHFUN_NANDXOR
	HASHINT shifto = HASHROTL(oval,1);
	const HASHINT ovalup = HASHCROP( shifto ^ ~(oval & mval[level]) );

	if(echo)
		Log("level%u m%u up%u o%u os%u", (int)level, (int)mval[level], (int)ovalup, (int)oval, (int)shifto);
#endif

#ifdef HASHFUN_NAND
	HASHINT shifto = HASHROTL(oval,1);
	HASHINT shiftm = HASHROTL(mval[level],1);

	HASHINT ovalup = HASHCROP( ~(oval & mval[level]) );
	ovalup = HASHCROP( ~(ovalup & shifto) );
	ovalup = HASHCROP( ~(ovalup & shiftm) );
#endif

#ifdef HASHFUN_NAND2
	HASHINT shifto = HASHROTL(oval,1);
	//HASHINT shiftm = HASHROTL(mval[level],1);

	HASHINT ovalup = HASHCROP( ~(oval & mval[level]) );
	ovalup = HASHCROP( ~(ovalup & shifto) );
	//ovalup = HASHCROP( ~(ovalup & shiftm) );
#endif

#ifdef HASHFUN_NANDNOR
	HASHINT shifto = HASHROTL(oval,1);
	//HASHINT shiftm = HASHROTL(mval[level],1);

	HASHINT ovalup = HASHCROP( ~(oval & mval[level]) );
	ovalup = HASHCROP( ~(ovalup | shifto) );
	//ovalup = HASHCROP( ~(ovalup & shiftm) );
#endif

#ifdef HASHFUN_NOR
	HASHINT shifto = HASHROTL(oval,1);
	//HASHINT shiftm = HASHROTL(mval[level],1);

	HASHINT ovalup = HASHCROP( ~(oval | mval[level]) );
	ovalup = HASHCROP( ~(ovalup | shifto) );
	//ovalup = HASHCROP( ~(ovalup & shiftm) );
#endif

#ifdef HASHFUN_XNOR
	HASHINT shifto = HASHROTL(oval,1);
	//HASHINT shiftm = HASHROTL(mval[level],1);

	HASHINT ovalup = HASHCROP( (~oval ^ ~mval[level]) );
	ovalup = HASHCROP( (~ovalup ^ ~shifto) );
	//ovalup = HASHCROP( ~(ovalup & shiftm) );
#endif

#define HASHFUN_NXOR

#ifdef HASHFUN_NXOR
	HASHINT shifto = HASHROTL(oval,1);
	//HASHINT shiftm = HASHROTL(mval[level],1);

	HASHINT ovalup = HASHCROP( ~(oval ^ mval[level]) );
	ovalup = HASHCROP( ~(ovalup ^ shifto) );
	//ovalup = HASHCROP( ~(ovalup & shiftm) );
#endif
	return Hash(mval, ovalup, level+1, echo);

}



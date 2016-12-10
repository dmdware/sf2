











#include "../platform.h"
#include "anim.h"

ecbool PlayAni(short* frame, short first, short last, ecbool loop, short rate)
{
	if(*frame < first || *frame > last+1)
	{
		*frame = first;
		return ecfalse;
	}

	(*frame) += ((g_simframe%RATIO_DENOM)<=rate);

	if(*frame > last)
	{
		if(loop)
			*frame = first;
		else
			*frame = last;

		return ectrue;
	}

	return ecfalse;
}

//Play animation backwards
ecbool PlayAniB(short* frame, short first, short last, ecbool loop, short rate)
{
	if(*frame < first-1 || *frame > last)
	{
		*frame = last;
		return ecfalse;
	}

	(*frame) -= ((g_simframe%RATIO_DENOM)<=rate);

	if(*frame < first)
	{
		if(loop)
			*frame = last;
		else
			*frame = first;

		return ectrue;
	}

	return ecfalse;
}
#include "platform.h"
#include "utils.h"

/* only use with dynamic array string */

void pstrset(char **out, char *in)
{
	int len = strlen(in);
	
	*out = (char*)malloc(len+1);
	memcpy(*out, in, len+1);
}

void pstradd(char **out, char *in)
{
	int addlen = strlen(in);
	int len = 0;
	
	len = strlen(*out);
	*out = (char*)realloc(*out, len + addlen + 1);
	
	memcpy(&(*out)[len], in, addlen + 1);
}

void psubstr(char **out, char *in, int beg, int len)
{
	*out = (char*)malloc(len + 1);
	memcpy(*out, &in[beg], len);
	(*out)[len] = 0;
}

void delprev(char **s, int *caret)
{
	int glyphi, ci, adv;
	unsigned int k;
	char *sub1, *sub2;
	
	for(glyphi=0, ci=0; (*s)[ci]; ci+=adv, glyphi++)
	{
		adv = 0;
		k = ToGlyph(&(*s)[ci], &adv);
		
		if(ci > 0 && adv + ci >= *caret)
		{
			psubstr(&sub1, *s, 0, ci);
			psubstr(&sub2, *s, adv+ci, strlen(*s)-adv-ci);
			free(*s);
			pstrset(s, sub1);
			pstradd(s, sub2);
			free(sub1);
			free(sub2);
			(*caret) -= (*caret)-ci;
			return;
		}
	}
}

void delnext(char **s, int *caret)
{
	int glyphi, ci, adv;
	unsigned int k;
	char *sub1, *sub2;
	
	for(glyphi=0, ci=0; (*s)[ci]; ci+=adv, glyphi++)
	{
		adv = 0;
		k = ToGlyph(&(*s)[ci], &adv);
		
		if(adv + ci > *caret)
		{
			psubstr(&sub1, *s, 0, ci);
			psubstr(&sub2, *s, adv+ci, strlen(*s)-adv-ci);
			free(*s);
			pstrset(s, sub1);
			pstradd(s, sub2);
			free(sub1);
			free(sub2);
			(*caret) -= (*caret)-ci;
			return;
		}
	}
}

int prevlen(char *s, int caret)
{
	int glyphi, ci, adv;
	unsigned int k;
	
	for(glyphi=0, ci=0; s[ci]; ci+=adv, glyphi++)
	{
		adv = 0;
		k = ToGlyph(&s[ci], &adv);
		
		if(ci > 0 && adv + ci >= caret)
			return adv;
	}
	
	return 0;
}

int nextlen(char *s, int caret)
{
	int glyphi, ci, adv;
	unsigned int k;
	
	for(glyphi=0, ci=0; s[ci]; ci+=adv, glyphi++)
	{
		adv = 0;
		k = ToGlyph(&s[ci], &adv);
		
		if(adv + ci > caret)
			return adv;
	}
	
	return 0;
}
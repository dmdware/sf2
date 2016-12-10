













#include "../trigger/condition.h"
#include "savecondition.h"
#include "../platform.h"

void SaveCondition(Condition* c, FILE* fp)
{
	fwrite(c->name, sizeof(char), CONDITION_LEN+1, fp);
	fwrite(&c->type, sizeof(int), 1, fp);
	fwrite(&c->met, sizeof(ecbool), 1, fp);
	fwrite(&c->target, sizeof(int), 1, fp);
	fwrite(&c->fval, sizeof(float), 1, fp);
	fwrite(&c->count, sizeof(int), 1, fp);
}

void ReadCondition(Condition* c, FILE* fp)
{
	fread(c->name, sizeof(char), CONDITION_LEN+1, fp);
	fread(&c->type, sizeof(int), 1, fp);
	fread(&c->met, sizeof(ecbool), 1, fp);
	fread(&c->target, sizeof(int), 1, fp);
	fread(&c->fval, sizeof(float), 1, fp);
	fread(&c->count, sizeof(int), 1, fp);
}
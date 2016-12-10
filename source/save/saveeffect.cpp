













#include "saveeffect.h"
#include "../platform.h"
#include "../trigger/effect.h"
#include "../trigger/trigger.h"

void SaveEffect(Effect* e, FILE* fp)
{
	fwrite(e->name, sizeof(char), CONDITION_LEN+1, fp);
	fwrite(&e->type, sizeof(int), 1, fp);
	int trigger = TriggerID(e->trigger);
	fwrite(&trigger, sizeof(int), 1, fp);
	int tvlen = strlen(e->textval.c_str())+1;
	fwrite(&tvlen, sizeof(int), 1, fp);
	fwrite(e->textval.c_str(), sizeof(char), tvlen, fp);
	fwrite(&e->imgdw, sizeof(int), 1, fp);
	fwrite(&e->imgdh, sizeof(int), 1, fp);
}

void ReadEffect(Effect* e, std::vector<int>* triggerrefs, FILE* fp)
{
	fread(e->name, sizeof(char), CONDITION_LEN+1, fp);
	fread(&e->type, sizeof(int), 1, fp);
	int ref;
	fread(&ref, sizeof(int), 1, fp);
	triggerrefs->push_back(ref);
	int tvlen = 0;
	fread(&tvlen, sizeof(int), 1, fp);
	char* tval = new char[tvlen];
	fread(tval, sizeof(char), tvlen, fp);
	e->textval = tval;
	delete [] tval;
	fread(&e->imgdw, sizeof(int), 1, fp);
	fread(&e->imgdh, sizeof(int), 1, fp);
}














#include "savehint.h"
#include "../trigger/hint.h"

void SaveHint(Hint* h, FILE* fp)
{
	int len = strlen(h->message.c_str())+1;
	fwrite(&len, sizeof(int), 1, fp);
	fwrite(h->message.c_str(), sizeof(char), len, fp);
	len = strlen(h->graphic.c_str())+1;
	fwrite(&len, sizeof(int), 1, fp);
	fwrite(h->graphic.c_str(), sizeof(char), len, fp);
	fwrite(&h->gwidth, sizeof(int), 1, fp);
	fwrite(&h->gheight, sizeof(int), 1, fp);
}

void ReadHint(Hint* h, FILE* fp)
{
	int len;
	fread(&len, sizeof(int), 1, fp);
	char* buff = new char[len];
	fread(buff, sizeof(char), len, fp);
	h->message = buff;
	delete [] buff;
	fread(&len, sizeof(int), 1, fp);
	buff = new char[len];
	fread(buff, sizeof(char), len, fp);
	h->graphic = buff;
	delete [] buff;
	fread(&h->gwidth, sizeof(int), 1, fp);
	fread(&h->gheight, sizeof(int), 1, fp);
}

void SaveLastHint(FILE* fp)
{
}

void ReadLastHint(FILE* fp)
{
}
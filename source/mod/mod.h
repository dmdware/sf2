

#ifndef MOD_H
#define MOD_H

#include "../gui/richtext.h"

#define MODNAME_LEN	64

extern char g_modname[MODNAME_LEN+1];
extern RichText g_abortmess;
extern int g_moderrs;

void LoadMod(const char* folder);
void LoadMod();
void AbortMod(const char* m);
void UnloadMod();

#endif
#ifndef USTR_H
#define USTR_H

#include "platform.h"

void pstrset(char **out, char *in);
void pstradd(char **out, char *in);
void psubstr(char **out, char *in, int beg, int len);
void delprev(char **s, int *caret);
void delnext(char **s, int *caret);
int prevlen(char *s, int caret);
int nextlen(char *s, int caret);

#endif

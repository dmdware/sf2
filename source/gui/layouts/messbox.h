












#ifndef MESSBOX_H
#define MESSBOX_H

struct RichText;

void FillMess();
void Mess(RichText* mess, void (*cfun)()=NULL);

extern void (*g_continuefun)();

#endif
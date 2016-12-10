











#ifndef CONSOLE_H
#define CONSOLE_H

#ifndef MATCHMAKER
#include "../gui/widgets/text.h"
#include "../gui/widgets/editbox.h"
#endif

#define CONSOLE_LINES	17

struct RichText;

void FillConsole();
void ToggleConsole();
void SubmitConsole(RichText* rt);

#endif
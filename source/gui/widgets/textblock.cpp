










#include "../widget.h"
#include "barbutton.h"
#include "button.h"
#include "checkbox.h"
#include "editbox.h"
#include "droplist.h"
#include "image.h"
#include "insdraw.h"
#include "link.h"
#include "listbox.h"
#include "text.h"
#include "textarea.h"
#include "textblock.h"
#include "touchlistener.h"



void TextBlock::draw()
{
	float width = pos[2] - pos[0];
	float height = pos[3] - pos[1];

	DrawBoxShadTextF(font, pos[0], pos[1], width, height, &text, rgba, 0, -1, crop[0], crop[1], crop[2], crop[3]);
	//DrawBoxTextF(font, pos[0], pos[1], width, height, &text, rgba, 0, -1, crop[0], crop[1], crop[2], crop[3]);

	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
}

void TextBlock::changevalue(const char* newv)
{
	value = newv;
	if(caret > strlen(newv))
		caret = strlen(newv);
	lines = CountLines(&value, MAINFONT8, pos[0], pos[1], pos[2]-pos[0]-square(), pos[3]-pos[1]);
}

int TextBlock::square()
{
	return (int)g_font[font].gheight;
}

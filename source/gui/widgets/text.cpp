










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

void Text::draw()
{
	//glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
	//glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0.8f, 0.8f, 0.8f, 1.0f);
	//float color[] = {0.8f, 0.8f, 0.8f, 1.0f};
	//DrawShadowedText(font, pos[0], pos[1], text.c_str(), color);

	//Log("draw text "<<text.rawstr().c_str()<<" (shadow: "<<shadow<<")");
	//

	if(shadow)
	{

#ifdef DEBUGLOG
		Log("text "<<__FILE__<<" "<<__LINE__);
		
#endif
#if 1
		//DrawShadowedTextF(font, pos[0], pos[1], pos[0], pos[1], pos[2], pos[3], &text, rgba);
		DrawShadowedTextF(font, pos[0], pos[1], crop[0], crop[1], crop[2], crop[3], &text, rgba);
#else
		DrawShadowedText(font, pos[0], pos[1], &text, rgba);
#endif
	}
	else
	{
#ifdef DEBUGLOG
		Log("text "<<__FILE__<<" "<<__LINE__);
		
#endif
		
		//DrawLineF(font, pos[0], pos[1], pos[0], pos[1], pos[2], pos[3], &text, rgba);
		DrawLineF(font, pos[0], pos[1], crop[0], crop[1], crop[2], crop[3], &text, rgba);
	}
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
}


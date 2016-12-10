










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
#include "../icon.h"
#include "../../sim/player.h"
#include "../../sys/unicode.h"

TextArea::TextArea(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), float r, float g, float b, float a, void (*change)()) : Widget()
{
	parent = parent;
	type = WIDGET_TEXTAREA;
	name = n;
	value = t;
	font = f;
	reframefunc = reframef;
	ldown = ecfalse;
	rgba[0] = r;
	rgba[1] = g;
	rgba[2] = b;
	rgba[3] = a;
	highl[0] = 0;
	highl[1] = 0;
	changefunc = change;
	scroll[1] = 0;
	highl[0] = 0;
	highl[1] = 0;
	caret = value.texlen();
	CreateTex(frametex, "gui/frame.jpg", ectrue, ecfalse);
	CreateTex(filledtex, "gui/filled.jpg", ectrue, ecfalse);
	CreateTex(uptex, "gui/up.jpg", ectrue, ecfalse);
	//CreateTex(downtex, "gui/down.jpg", ectrue, ecfalse);
	reframe();
	lines = CountLines(&value, f, pos[0], pos[1], pos[2]-pos[0]-square(), pos[3]-pos[1]);
}

void TextArea::draw()
{
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);

	DrawImage(frametex, pos[0], pos[1], pos[2], pos[3], 0,0,1,1, crop);

	DrawImage(g_texture[frametex].texname, pos[2]-square(), pos[1], pos[2], pos[3], 0,0,1,1, crop);
	DrawImage(g_texture[uptex].texname, pos[2]-square(), pos[1], pos[2], pos[1]+square(), 0,0,1,1, crop);
	DrawImage(g_texture[downtex].texname, pos[2]-square(), pos[3]-square(), pos[2], pos[3], 0,0,1,1, crop);
	DrawImage(g_texture[filledtex].texname, pos[2]-square(), pos[1]+square()+scrollspace()*topratio(), pos[2], pos[1]+square()+scrollspace()*bottomratio(), 0,0,1,1, crop);

	float width = pos[2] - pos[0] - square();
	float height = pos[3] - pos[1];

	//DrawBoxShadText(font, pos[0], pos[1], width, height, value.c_str(), rgba, scroll[1], opened ? caret : -1);

	DrawShadowedTextF(font, pos[0]+scroll[0], pos[1], pos[0], pos[1], pos[2], pos[3], &value, NULL, opened ? caret : -1);

	HighlightF(font, pos[0]+scroll[0], pos[1], pos[0], pos[1], pos[2], pos[3], &value, highl[0], highl[1]);
}

int TextArea::rowsshown()
{
	int rows = (int)( (pos[3]-pos[1])/g_font[font].gheight );

	return rows;
}

int TextArea::square()
{
	return (int)g_font[font].gheight;
}

float TextArea::scrollspace()
{
	return (pos[3]-pos[1]-square()*2);
}

void TextArea::inev(InEv* ie)
{
	Py* py = &g_py[g_localP];

	if(ie->type == INEV_MOUSEMOVE && !ie->intercepted)
	{
		if(ldown)
		{
			int newcaret = MatchGlyphF(&value, font, g_mouse.x, pos[0]+scroll[0], pos[1], pos[0], pos[1], pos[2], pos[3]);

			if(newcaret > caret)
			{
				highl[0] = caret;
				highl[1] = newcaret;
				//Log("hihgl "<<highl[0]<<"->"<<highl[1]);
				//
			}
			else
			{
				highl[0] = newcaret;
				highl[1] = caret;
				//Log("hihgl "<<highl[0]<<"->"<<highl[1]);
				//
			}

			ie->intercepted = ectrue;
			return;
		}

		if(g_mouse.x >= pos[0] && g_mouse.x <= pos[2] && g_mouse.y >= pos[1] && g_mouse.y <= pos[3])
		{
			over = ectrue;

			ie->intercepted = ectrue;
			return;
		}
		else
		{
			over = ecfalse;

			return;
		}
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(over)
		{
			ldown = ectrue;
			ie->intercepted = ectrue;
			return;
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		if(over && ldown)
		{
			ldown = ecfalse;
			ie->intercepted = ectrue;
			gainfocus();
			return;
		}

		ldown = ecfalse;

		if(opened)
		{
			opened = ecfalse;
			return;
		}
	}
	else if(ie->type == INEV_KEYDOWN && !ie->intercepted)
	{
		if(!opened)
			return;

		//int len = value.length();
		int len = value.texlen();

		if(caret > len)
			caret = len;

		if(ie->key == SDLK_LEFT)
		{
			if(caret <= 0)
			{
				ie->intercepted = ectrue;
				return;
			}

			caret --;
		}
		else if(ie->key == SDLK_RIGHT)
		{
			int len = value.texlen();

			if(caret >= len)
			{
				ie->intercepted = ectrue;
				return;
			}

			caret ++;
		}
		else if(ie->key == SDLK_DELETE)
		{
			len = value.texlen();

			if((highl[1] <= 0 || highl[0] == highl[1]) && caret >= len || len <= 0)
			{
				ie->intercepted = ectrue;
				return;
			}

			delnext();

			if(!passw)
				value = ParseTags(value, &caret);
		}
#if 0
		else if(ie->key == 190 && !g_keys[SDLK_SHIFT])
		{
			//placechar('.');
		}
#endif

		if(changefunc != NULL)
			changefunc();

		if(changefunc2 != NULL)
			changefunc2(param);

		ie->intercepted = ectrue;
	}
	else if(ie->type == INEV_KEYUP && !ie->intercepted)
	{
		if(!opened)
			return;

		ie->intercepted = ectrue;
	}
	else if(ie->type == INEV_CHARIN && !ie->intercepted)
	{
		if(!opened)
			return;

		int len = value.texlen();

		if(caret > len)
			caret = len;

		if(ie->key == SDLK_BACKSPACE)
		{
			len = value.texlen();

			if((highl[1] <= 0 || highl[0] == highl[1]) && caret >= len || len <= 0)
			{
				ie->intercepted = ectrue;
				return;
			}

			delprev();

			if(!passw)
				value = ParseTags(value, &caret);
		}/*
		 else if(k == SDLK_DELETE)
		 {
		 len = value.texlen();

		 if((highl[1] <= 0 || highl[0] == highl[1]) && caret >= len || len <= 0)
		 return ectrue;

		 delnext();

		 if(!passw)
		 value = ParseTags(value, &caret);
		 }*/
		else if(ie->key == SDLK_LSHIFT || ie->key == SDLK_RSHIFT)
		{
			ie->intercepted = ectrue;
			return;
		}
		else if(ie->key == SDLK_CAPSLOCK)
		{
			ie->intercepted = ectrue;
			return;
		}
		else if(ie->key == SDLK_SPACE)
		{
			placechar(' ');
		}
		else if(ie->key == SDLK_RETURN)
		{
			placechar('\n');
		}
		//else if(k >= 'A' && k <= 'Z' || k >= 'a' && k <= 'z')
		//else if(k == 190 || k == 188)
		//else if((k >= '!' && k <= '@') || (k >= '[' && k <= '`') || (k >= '{' && k <= '~') || (k >= '0' || k <= '9'))
		else
		{

#ifdef PASTE_DEBUG
			Log("charin "<<(char)k<<" ("<<k<<")");
			
#endif
#if 0
			//if(k == 'C' && g_keys[SDLK_CONTROL])
			if(ie->key == 3)	//copy
			{
				copyval();
			}
			//else if(k == 'V' && g_keys[SDLK_CONTROL])
			else if(ie->key == 22)	//paste
			{
				pasteval();
			}
			//else if(k == 'A' && g_keys[SDLK_CONTROL])
			else if(ie->key == 1)	//select all
			{
				selectall();
			}
			else
#endif
				placechar(ie->key);
		}

		if(changefunc != NULL)
			changefunc();

		if(changefunc2 != NULL)
			changefunc2(param);

		ie->intercepted = ectrue;
	}
	else if(ie->type == INEV_TEXTIN && !ie->intercepted)
	{
		if(!opened)
			return;

		ie->intercepted = ectrue;

		int len = value.texlen();

		if(caret > len)
			caret = len;

		//Log("vk "<<ie->key);
		//


#if 0
		if(ie->key == SDLK_SPACE)
		{
			placechar(' ');
		}
		else
#endif

#ifdef PASTE_DEBUG
			Log("charin "<<(char)ie->key<<" ("<<ie->key<<")");
		
#endif

#if 0
		//if(ie->key == 'C' && g_keys[SDLK_CONTROL])
		if(ie->key == 3)	//copy
		{
			copyval();
		}
		//else if(ie->key == 'V' && g_keys[SDLK_CONTROL])
		else if(ie->key == 22)	//paste
		{
			pasteval();
		}
		//else if(ie->key == 'A' && g_keys[SDLK_CONTROL])
		else if(ie->key == 1)	//select all
		{
			selectall();
		}
		else
#endif
		//unsigned int* ustr = ToUTF32((const unsigned char*)ie->text.c_str(), ie->text.length());
		unsigned int* ustr = ToUTF32((const unsigned char*)ie->text.c_str());
		//RichText addstr(RichPart(UStr(ustr)));	//Why does MSVS2012 not accept this?
		RichText addstr = RichText(RichPart(UStr(ustr)));
		unsigned int first = ustr[0];
		delete [] ustr;

		placestr(&addstr);

		if(changefunc != NULL)
			changefunc();

		if(changefunc2 != NULL)
			changefunc2(param);

		//if(changefunc3 != NULL)
		//	changefunc3(first, 0, ectrue, param);

		ie->intercepted = ectrue;
	}
	else if(ie->type == INEV_PASTE && !ie->intercepted)
	{
		if(!opened)
			return;

		ie->intercepted = ectrue;

		int len = value.texlen();

		if(caret > len)
			caret = len;

		pasteval();
	}
	else if(ie->type == INEV_COPY && !ie->intercepted)
	{
		if(!opened)
			return;

		ie->intercepted = ectrue;

		int len = value.texlen();

		if(caret > len)
			caret = len;

		copyval();
	}
	else if(ie->type == INEV_SELALL && !ie->intercepted)
	{
		if(!opened)
			return;

		ie->intercepted = ectrue;

		int len = value.texlen();

		if(caret > len)
			caret = len;

		selectall();
	}
}

void TextArea::changevalue(const char* newv)
{
	value = newv;
	if(caret > strlen(newv))
		caret = strlen(newv);
	lines = CountLines(&value, MAINFONT8, pos[0], pos[1], pos[2]-pos[0]-square(), pos[3]-pos[1]);
}

#if 0
void TextArea::placestr(const char* str)
{
	int len = value.texlen();

	if(highl[1] > 0 && highl[0] != highl[1])
	{
		len -= highl[1] - highl[0];
	}

	int addlen = strlen(str);
	if(addlen + len >= maxlen)
		addlen = maxlen - len;

	unsigned int* addstr = ToUTF32((unsigned char*)str);

	if(!addstr)
		OUTOFMEM();

	if(addlen > 0)
	{
		addstr[addlen] = 0;
	}
	else
	{
		delete [] addstr;
		return;
	}

	if(highl[1] > 0 && highl[0] != highl[1])
	{
		RichText before = value.substr(0, highl[0]);
		//RichText after = value.substr(highl[1]-1, value.texlen()-highl[1]);
		RichText after = value.substr(highl[1], value.texlen()-highl[1]);
		value = before + RichText(UStr(addstr)) + after;

		caret = highl[0] + addlen;
		highl[0] = highl[1] = 0;
	}
	else
	{
		if(len >= maxlen)
		{
			delete [] addstr;
			return;
		}

		RichText before = value.substr(0, caret);
		RichText after = value.substr(caret, value.texlen()-caret);
		value = before + addstr + after;
		caret += addlen;
	}

	//RichText val = drawvalue();
	int endx = EndX(&value, caret, font, pos[0]+scroll[0], pos[1]);

	if(endx >= pos[2])
		scroll[0] -= endx - pos[2] + 1;

	delete [] addstr;
}
#else
void TextArea::placestr(const RichText* str)
{
	int len = value.texlen();

	if(highl[1] > 0 && highl[0] != highl[1])
	{
		len -= highl[1] - highl[0];
	}

	int addlen = str->texlen();
	if(addlen + len >= maxlen)
		addlen = maxlen - len;

	RichText addstr = str->substr(0, addlen);

	if(highl[1] > 0 && highl[0] != highl[1])
	{
		RichText before = value.substr(0, highl[0]);
		//RichText after = value.substr(highl[1]-1, value.texlen()-highl[1]);
		RichText after = value.substr(highl[1], value.texlen()-highl[1]);
		value = before + addstr + after;

		caret = highl[0] + addlen;
		highl[0] = highl[1] = 0;
	}
	else
	{
		if(len >= maxlen)
		{
			return;
		}

		RichText before = value.substr(0, caret);
		RichText after = value.substr(caret, value.texlen()-caret);
		value = before + addstr + after;
		caret += addlen;

		//LogRich(&value);
	}

	//RichText val = drawvalue();
	int endx = EndX(&value, caret, font, pos[0]+scroll[0], pos[1]);

	if(endx >= pos[2])
		scroll[0] -= endx - pos[2] + 1;
}
#endif

ecbool TextArea::delnext()
{
	int len = value.texlen();

	if(highl[1] > 0 && highl[0] != highl[1])
	{
		RichText before = value.substr(0, highl[0]);
		RichText after = value.substr(highl[1], len-highl[1]);
		value = before + after;

		caret = highl[0];
		highl[0] = highl[1] = 0;
	}
	else if(caret >= len || len <= 0)
		return ectrue;
	else
	{
		RichText before = value.substr(0, caret);
		RichText after = value.substr(caret+1, len-caret);
		value = before + after;
	}

	//RichText val = drawvalue();
	int endx = EndX(&value, caret, font, pos[0]+scroll[0], pos[1]);

	if(endx <= pos[0])
		scroll[0] += pos[0] - endx + 1;
	else if(endx >= pos[2])
		scroll[0] -= endx - pos[2] + 1;

	highl[0] = highl[1] = 0;

	return ectrue;
}

ecbool TextArea::delprev()
{
	int len = value.texlen();

	if(highl[1] > 0 && highl[0] != highl[1])
	{
		RichText before = value.substr(0, highl[0]);
		RichText after = value.substr(highl[1], len-highl[1]);
		value = before + after;

		caret = highl[0];
		highl[0] = highl[1] = 0;
	}
	else if(caret <= 0 || len <= 0)
		return ectrue;
	else
	{
		RichText before = value.substr(0, caret-1);
		RichText after = value.substr(caret, len-caret);
		value = before + after;

		caret--;
	}

	//RichText val = drawvalue();
	int endx = EndX(&value, caret, font, pos[0]+scroll[0], pos[1]);

	if(endx <= pos[0])
		scroll[0] += pos[0] - endx + 1;
	else if(endx >= pos[2])
		scroll[0] -= endx - pos[2] + 1;

	highl[0] = highl[1] = 0;

	return ectrue;
}

//#define PASTE_DEBUG

void TextArea::copyval()
{
#ifdef PASTE_DEBUG
	Log("copy vkc");
	
#endif

#if 0
	if(highl[1] > 0 && highl[0] != highl[1])
	{
		RichText highl = value.substr(highl[0], highl[1]-highl[0]);
		std::string rawhighl = highl.rawstr();
		const size_t len = strlen(rawhighl.c_str())+1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), rawhighl.c_str(), len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}
	else
	{
		const char* output = "";
		const size_t len = strlen(output)+1;
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
		memcpy(GlobalLock(hMem), output, len);
		GlobalUnlock(hMem);
		OpenClipboard(0);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hMem);
		CloseClipboard();
	}

	//return ectrue;
#else
	if(highl[1] > 0 && highl[0] != highl[1])
	{
		RichText highl = value.substr(highl[0], highl[1]-highl[0]);
		SDL_SetClipboardText( highl.rawstr().c_str() );
	}
	else
	{
		SDL_SetClipboardText( "" );
	}
#endif
}

void TextArea::pasteval()
{
#if 0
	placestr( SDL_GetClipboardText() );
#else
	unsigned int* ustr = ToUTF32( (unsigned char*)SDL_GetClipboardText() ); 
	RichText rstr = RichText(UStr(ustr));
	delete [] ustr;
	placestr( &rstr );
#endif

	if(!passw)
		value = ParseTags(value, &caret);
}

void TextArea::placechar(unsigned int k)
{
	//int len = value.length();

	//if(type == WIDGET_EDITBOX && len >= maxlen)
	//	return;

	//char addchar = k;

	//std::string before = value.substr(0, caret);
	//std::string after = value.substr(caret, len-caret);
	//value = before + addchar + after;

	RichText newval;

	int currplace = 0;
	ecbool changed = ecfalse;
	for(std::list<RichPart>::iterator i=value.part.begin(); i!=value.part.end(); i++)
	{
		if(currplace + i->texlen() >= caret && !changed)
		{
			changed = ectrue;

			if(i->type == RICH_TEXT)
			{
				if(i->text.length <= 1)
					continue;

				RichPart chpart;

				chpart.type = RICH_TEXT;

				int subplace = caret - currplace;

				if(subplace > 0)
				{
					chpart.text = chpart.text + i->text.substr(0, subplace);
				}

				chpart.text = chpart.text + UStr(k);

				if(i->text.length - subplace > 0)
				{
					chpart.text = chpart.text + i->text.substr(subplace, i->text.length-subplace);
				}

				chpart.text = i->text.substr(0, i->text.length-1);

				newval = newval + RichText(chpart);
			}
			else if(i->type == RICH_ICON)
			{
				Icon* icon = &g_icon[i->icon];

				int subplace = caret - currplace;

				if(subplace <= 0)
				{
					newval = newval + RichText(RichPart(UStr(k)));
					newval = newval + RichText(*i);
				}
				else
				{
					newval = newval + RichText(*i);
					newval = newval + RichText(RichPart(UStr(k)));
				}
			}

		}
		else
		{
			newval = newval + RichText(*i);
			currplace += i->texlen();
		}
	}

	value = newval;

	caret ++;
}

void TextArea::selectall()
{
	highl[0] = 0;
	highl[1] = value.texlen()+1;
	caret = -1;

	//RichText val = drawvalue();
	int endx = EndX(&value, value.texlen(), font, pos[0]+scroll[0], pos[1]);

	if(endx <= pos[2])
		scroll[0] += pos[2] - endx - 1;

	if(scroll[0] >= 0)
		scroll[0] = 0;

	//return ectrue;
}

void TextArea::hide()
{
	losefocus();

	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		(*i)->hide();
}


void TextArea::gainfocus()
{
	if(!opened)
	{
		Py* py = &g_py[g_localP];

		if(g_kbfocus > 0)
		{
			SDL_StopTextInput();
			g_kbfocus--;
		}

		opened = ectrue;
		SDL_StartTextInput();
		SDL_Rect r;
		r.x = (int)pos[0];
		r.y = (int)pos[3];
		r.w = g_width - (int)pos[0];
		r.h = g_height - (int)pos[3];
		SDL_SetTextInputRect(&r);
		g_kbfocus++;
	}
}

void TextArea::losefocus()
{
	if(opened)
	{
		Py* py = &g_py[g_localP];

		if(g_kbfocus > 0)
		{
			SDL_StopTextInput();
			g_kbfocus--;
		}

		opened = ecfalse;
	}
}












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
#include "../../sim/player.h"
#include "../gui.h"
#include "../../sys/unicode.h"
#include "../../debug.h"

EditBox::EditBox() : Widget()
{
	parent = NULL;
	type = WIDGET_EDITBOX;
	name = "";
	font = MAINFONT8;
	value = "";
	caret = value.texlen();
	opened = ecfalse;
	passw = ecfalse;
	maxlen = 0;
	reframefunc = NULL;
	submitfunc = NULL;
	changefunc = NULL;
	changefunc2 = NULL;
	changefunc3 = NULL;
	scroll[0] = 0;
	highl[0] = 0;
	highl[1] = 0;
	CreateTex(frametex, "gui/frame.jpg", ectrue, ecfalse);
	param = -1;
	changefunc2 = NULL;
	//reframe();
}

EditBox::EditBox(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), ecbool pw, int maxl, void (*change3)(unsigned int key, unsigned int scancode, ecbool down, int parm), void (*submitf)(), int parm) : Widget()
{
	parent = parent;
	type = WIDGET_EDITBOX;
	name = n;
	font = f;
	value = t;
	caret = value.texlen();
	opened = ecfalse;
	passw = pw;
	maxlen = maxl;
	reframefunc = reframef;
	submitfunc = submitf;
	changefunc = NULL;
	changefunc2 = NULL;
	changefunc3 = change3;
	scroll[0] = 0;
	highl[0] = 0;
	highl[1] = 0;
	CreateTex(frametex, "gui/frame.jpg", ectrue, ecfalse);
	param = parm;
	changefunc2 = NULL;
	reframe();
}

RichText EditBox::drawvalue()
{
	/*
	std::string val = value;

	if(passw)
	{
		val = "";
		for(int i=0; i<value.length(); i++)
			val.append("*");
	}

	return val;*/

	if(!passw)
		return value;

	return value.pwver();
}

void EditBox::draw()
{
	//glColor4f(1, 1, 1, 1);
	glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);

	DrawImage(g_texture[frametex].texname, pos[0], pos[1], pos[2], pos[3], 0,0,1,1, crop);

	if(over)
		//glColor4f(1, 1, 1, 1);
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);
	else
		//glColor4f(0.8f, 0.8f, 0.8f, 1);
		glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 0.8f, 0.8f, 0.8f, 1);

	RichText val = drawvalue();

	//if(opened)
	//	Log("op caret="<<caret);

	DrawShadowedTextF(font, pos[0]+scroll[0], pos[1], pos[0], pos[1], pos[2], pos[3], &val, NULL, opened ? caret : -1);

	//glColor4f(1, 1, 1, 1);
	//glUniform4f(g_sh[SHADER_ORTHO].slot[SSLOT_COLOR], 1, 1, 1, 1);

	HighlightF(font, pos[0]+scroll[0], pos[1], pos[0], pos[1], pos[2], pos[3], &val, highl[0], highl[1]);
}

//#define MOUSESC_DEBUG

void EditBox::frameupd()
{
#ifdef MOUSESC_DEBUG
	Log("editbox frameup");
	
#endif

	Py* py = &g_py[g_localP];

	if(ldown)
	{
		ecbool movedcar = ecfalse;

#ifdef MOUSESC_DEBUG
		Log("ldown frameup");
		
#endif

		if(g_mouse.x >= pos[2]-5)
		{
			scroll[0] -= fmax(1, g_font[font].gheight/4.0f);

			RichText val = drawvalue();
			int vallen = val.texlen();

			int endx = EndX(&val, vallen, font, pos[0]+scroll[0], pos[1]);

			if(endx < pos[2])
				scroll[0] += pos[2] - endx;

			if(scroll[0] > 0.0f)
				scroll[0] = 0.0f;

			movedcar = ectrue;
		}
		else if(g_mouse.x <= pos[0]+5)
		{
			scroll[0] += fmax(1, g_font[font].gheight/4.0f);

			if(scroll[0] > 0.0f)
				scroll[0] = 0.0f;

			movedcar = ectrue;
		}

		if(movedcar)
		{
			RichText val = drawvalue();
			int newcaret = MatchGlyphF(&val, font, g_mouse.x, pos[0]+scroll[0], pos[1], pos[0], pos[1], pos[2], pos[3]);

			if(newcaret > caret)
			{
				highl[0] = caret;
				highl[1] = newcaret;
			}
			else if(newcaret < caret)
			{
				highl[0] = newcaret;
				highl[1] = caret;
			}
		}
	}
}

void EditBox::inev(InEv* ie)
{
//#ifdef MOUSESC_DEBUG
	//Log("editbox mousemove");
	//
//#endif

	Py* py = &g_py[g_localP];

	if(ie->type == INEV_MOUSEMOVE)
	{
		if(!ie->intercepted)
		{
			if(ldown)
			{
				RichText val = drawvalue();
				int newcaret = MatchGlyphF(&val, font, g_mouse.x, pos[0]+scroll[0], pos[1], pos[0], pos[1], pos[2], pos[3]);

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

				g_mouseoveraction = ectrue;

				ie->intercepted = ectrue;
				return;
			}
			else
			{
				over = ecfalse;

				return;
			}
		}
	}
	else if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT)
	{
		if(opened)
		{
			opened = ecfalse;
			highl[0] = highl[1] = 0;
		}

		if(!ie->intercepted)
		{
			if(over)
			{
				ldown = ectrue;

				RichText val = drawvalue();

				//highl[1] = MatchGlyphF(value.c_str(), font, pos[0]+scroll[0], pos[1], pos[0], pos[1], pos[2], pos[3]);
				//highl[0] = highl[1];
				//caret = highl[1];
				caret = MatchGlyphF(&val, font, g_mouse.x, pos[0]+scroll[0], pos[1], pos[0], pos[1], pos[2], pos[3]);

				highl[0] = 0;
				highl[1] = 0;

				ie->intercepted = ectrue;
				return;
			}
		}
	}
	else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT && !ie->intercepted)
	{
		//if(over && ldown)
		if(ldown)
		{
			ldown = ecfalse;

			if(highl[1] > 0 && highl[0] != highl[1])
			{
				caret = -1;
			}

			ie->intercepted = ectrue;
			gainfocus();

			return;
		}

		ldown = ecfalse;

		if(opened)
		{
			ie->intercepted = ectrue;
			return;
		}
	}
	else if(ie->type == INEV_KEYDOWN && !ie->intercepted)
	{
		if(!opened)
			return;

		int len = value.texlen();

		if(caret > len)
			caret = len;

		if(ie->key == SDLK_F1)
			return;

		if(ie->key == SDLK_LEFT)
		{
			if(highl[0] > 0 && highl[0] != highl[1])
			{
				caret = highl[0];
				highl[0] = highl[1] = 0;
			}
			else if(caret <= 0)
			{
				ie->intercepted = ectrue;
				return;
			}
			else
				caret --;

			RichText val = drawvalue();
			int endx = EndX(&val, caret, font, pos[0]+scroll[0], pos[1]);

			//Log("left endx = "<<endx<<"/"<<pos[0]);
			//

			if(endx <= pos[0])
				scroll[0] += pos[0] - endx + 1;
		}
		else if(ie->key == SDLK_RIGHT)
		{
			int len = value.texlen();

			if(highl[0] > 0 && highl[0] != highl[1])
			{
				caret = highl[1];
				highl[0] = highl[1] = 0;
			}
			else if(caret >= len)
			{
				ie->intercepted = ectrue;
				return;
			}
			else
				caret ++;

			RichText val = drawvalue();
			int endx = EndX(&val, caret, font, pos[0]+scroll[0], pos[1]);

			if(endx >= pos[2])
				scroll[0] -= endx - pos[2] + 1;
		}
		else if(ie->key == SDLK_DELETE)
		{
			len = value.texlen();

			//Log("vk del");
			//

			if((highl[1] <= 0 || highl[0] == highl[1]) && caret >= len || len <= 0)
			{
				ie->intercepted = ectrue;
				return;
			}

			delnext();

			if(!passw)
				value = ParseTags(value, &caret);
		}
		if(ie->key == SDLK_BACKSPACE)
		{
			len = value.texlen();
			//len = value.rawstr().length();

			if((highl[1] <= 0 || highl[0] == highl[1]) && len <= 0)
			{
				ie->intercepted = ectrue;
				return;
			}

			delprev();

			if(!passw)
				value = ParseTags(value, &caret);
		}/*
		 else if(ie->key == SDLK_DELETE)
		 {
		 len = value.texlen();

		 Log("vk del");
		 

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
		else if(ie->key == SDLK_RETURN || ie->key == SDLK_RETURN2)
		{
			ie->intercepted = ectrue;
			if(submitfunc)
				submitfunc();
			return;
		}
#if 0
		else if(ie->key == 190 && !g_keys[SDLK_SHIFT])
		{
			//placechar('.');
		}
#endif

		if(changefunc2 != NULL)
			changefunc2(param);

		if(changefunc3 != NULL)
			changefunc3(ie->key, ie->scancode, ectrue, param);

		ie->intercepted = ectrue;
	}
	else if(ie->type == INEV_KEYUP && !ie->intercepted)
	{
		if(!opened)
			return;

		if(changefunc3 != NULL)
			changefunc3(ie->key, ie->scancode, ecfalse, param);

		ie->intercepted = ectrue;
	}
	else if(ie->type == INEV_TEXTIN && !ie->intercepted)
	{
		if(!opened)
			return;

		ie->intercepted = ectrue;

		int len = value.texlen();
		//len = value.rawstr().length();

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

		if(changefunc3 != NULL)
			changefunc3(first, 0, ectrue, param);

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

void EditBox::placestr(const RichText* str)
{
	int len = value.texlen();
	int rawlen = value.rawstr().length();

	if(highl[1] > 0 && highl[0] != highl[1])
	{
		len -= highl[1] - highl[0];
	}

	//corpc fix's all around texlen();

	int addlen = str->texlen();
	int rawaddlen = str->rawstr().length();
	if(rawaddlen + rawlen >= maxlen)
		//addlen = maxlen - len;
		rawaddlen = maxlen - rawlen;

	/*
	we want to make sure that the UTF8 string
	will be below maxlen, with the icon tags
	shown, NOT the final resulting RichText string.
	*/
	//RichText addstr = str->substr(0, addlen);
	std::string rawaddstr = str->rawstr().substr(0, rawaddlen);
	unsigned int* rawaddustr = ToUTF32((unsigned char*)rawaddstr.c_str());
	RichText addstr = RichText(UStr(rawaddustr));
	delete [] rawaddustr;

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

	if(!passw)
		value = ParseTags(value, &caret);

	RichText val = drawvalue();
	int endx = EndX(&val, caret, font, pos[0]+scroll[0], pos[1]);

	if(endx >= pos[2])
		scroll[0] -= endx - pos[2] + 1;
}

#if 0
void EditBox::changevalue(const char* str)
{
	int len = value.texlen();

	if(len >= maxlen)
		return;

	int setlen = strlen(str);
	if(setlen >= maxlen)
		setlen = maxlen;

	char* setstr = new char[setlen+1];

	if(!setstr)
		OUTOFMEM();

	for(int i=0; i<setlen; i++)
		setstr[i] = str[i];
	setstr[setlen] = '\0';

	value = setstr;
	highl[0] = highl[1] = 0;
	caret = 0;

	delete [] setstr;
}
#else

void EditBox::changevalue(const RichText* str)
{
#if 0
	int setlen = str->texlen();
	if(setlen >= maxlen)
		setlen = maxlen;

	value = str->substr(0, setlen);
	highl[0] = highl[1] = 0;
	caret = 0;
#else
	caret = 0;
	highl[0] = highl[1] = 0;
	value = RichText();
	placestr(str);	//does rawlen checks here
#endif
}
#endif

ecbool EditBox::delnext()
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

	RichText val = drawvalue();
	int endx = EndX(&val, caret, font, pos[0]+scroll[0], pos[1]);

	if(endx <= pos[0])
		scroll[0] += pos[0] - endx + 1;
	else if(endx >= pos[2])
		scroll[0] -= endx - pos[2] + 1;

	highl[0] = highl[1] = 0;

	return ectrue;
}

ecbool EditBox::delprev()
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

		//Log("before newval="<<before.rawstr()<<" texlen="<<before.texlen());
		//Log("after="<<after.rawstr()<<" texlen="<<after.texlen());
		//Log("ba newval="<<value.rawstr()<<" texlen="<<(before + after).texlen());
		//Log("newval="<<value.rawstr()<<" texlen="<<value.texlen());

		caret--;
	}

	RichText val = drawvalue();
	int endx = EndX(&val, caret, font, pos[0]+scroll[0], pos[1]);

	if(endx <= pos[0])
		scroll[0] += pos[0] - endx + 1;
	else if(endx >= pos[2])
		scroll[0] -= endx - pos[2] + 1;

	highl[0] = highl[1] = 0;

	return ectrue;
}

//#define PASTE_DEBUG

void EditBox::copyval()
{
#ifdef PASTE_DEBUG
	Log("copy vkc");
	
#endif

	if(highl[1] > 0 && highl[0] != highl[1])
	{
		RichText highl = value.substr(highl[0], highl[1]-highl[0]);
		SDL_SetClipboardText( highl.rawstr().c_str() );
	}
	else
	{
		SDL_SetClipboardText( "" );
	}

	//return ectrue;
}

void EditBox::pasteval()
{
	unsigned int* ustr = ToUTF32( (unsigned char*)SDL_GetClipboardText() ); 
	RichText rstr = RichText(UStr(ustr));
	delete [] ustr;
	placestr( &rstr );

	if(!passw)
		value = ParseTags(value, &caret);
}

void EditBox::selectall()
{
	highl[0] = 0;
	highl[1] = value.texlen()+1;
	caret = -1;

	RichText val = drawvalue();
	int endx = EndX(&val, value.texlen(), font, pos[0]+scroll[0], pos[1]);

	if(endx <= pos[2])
		scroll[0] += pos[2] - endx - 1;

	if(scroll[0] >= 0)
		scroll[0] = 0;

	//return ectrue;
}

void EditBox::hide()
{
	losefocus();

	for(std::list<Widget*>::iterator i=sub.begin(); i!=sub.end(); i++)
		(*i)->hide();
}


void EditBox::gainfocus()
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
#if 0
		r.x = (int)pos[0];
		r.y = (int)pos[3];
		r.w = (int)(g_width - pos[0]);
		r.h = (int)(g_height - pos[3]);
#else
		r.x = (int)g_width/2;
		r.y = (int)0;
		r.w = (int)g_width/2;
		r.h = (int)g_height;
#endif
		SDL_SetTextInputRect(&r);
		g_kbfocus++;
	}
}

void EditBox::losefocus()
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

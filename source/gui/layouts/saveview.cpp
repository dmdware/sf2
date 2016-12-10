











#include "../../../platform.h"
#include "../../widget.h"
#include "../barbutton.h"
#include "../button.h"
#include "../checkbox.h"
#include "../editbox.h"
#include "../droplist.h"
#include "../image.h"
#include "../insdraw.h"
#include "../link.h"
#include "../listbox.h"
#include "../text.h"
#include "../textarea.h"
#include "../textblock.h"
#include "../touchlistener.h"
#include "../frame.h"
#include "cstrview.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../layouts/appviewport.h"
#include "../../../sim/building.h"
#include "../../../sim/bltype.h"
#include "../../../sim/unit.h"
#include "../../../sim/utype.h"
#include "../../../sim/player.h"
#include "svlist.h"
#include "blpreview.h"
#include "saveview.h"
#include "loadview.h"
#include "../../icon.h"
#include "../../../net/sendpackets.h"
#include "../../../save/savemap.h"
#include "../../layouts/appgui.h"
#include "../../../language.h"

const char *SAVEMODEPATH[SAVEMODE_TYPES] = {"saves/", "saves/"};

//save view save button
void Resize_SV_SaveBut(Widget* w)
{
	SaveView* parw = (SaveView*)w->m_parent;

	w->m_pos[0] = parw->m_pos[2] - 130;
	w->m_pos[1] = parw->m_pos[3] - 30;
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = parw->m_pos[3];

	CenterLabel(w);
}

//save file current name text label
void Resize_SV_CurName(Widget* w)
{
	SaveView* parw = (SaveView*)w->m_parent;

	w->m_pos[0] = parw->m_pos[0] + 30;
	w->m_pos[1] = parw->m_pos[3] - 60;
	w->m_pos[2] = parw->m_pos[2];
	w->m_pos[3] = w->m_pos[1] + 16;

	CenterLabel(w);
}

//save view delete button
void Resize_SV_DelBut(Widget* w)
{
	SaveView* parw = (SaveView*)w->m_parent;

	w->m_pos[0] = parw->m_pos[0];
	w->m_pos[1] = parw->m_pos[3] - 30;
	w->m_pos[2] = parw->m_pos[0] + 130;
	w->m_pos[3] = parw->m_pos[3];

	CenterLabel(w);
}

void Click_SV_Save()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SaveView* v = (SaveView*)gui->get("save");
	EditBox* curn = (EditBox*)&v->m_curname;

	std::string path = std::string(SAVEMODEPATH[v->m_savemode]) + curn->m_value.rawstr();
	SaveMap(path.c_str());

	strcpy(g_lastsave, path.c_str());

	gui->hide("save");
	gui->hide("load");
}


void Click_SV_Del()
{
	Player* py = &g_player[g_localP];
	GUI* gui = &g_gui;
	SaveView* v = (SaveView*)gui->get("save");

	if(!v->m_selfile)
		return;

	std::string path = std::string(SAVEMODEPATH[v->m_savemode]) + v->m_selfile->rawstr();
	char fullpath[WF_MAX_PATH+1];
	FullWritePath(path.c_str(), fullpath);
	unlink(fullpath);

	v->m_selfile = NULL;
	v->regen(v->m_savemode);
}

SaveView::SaveView(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_SAVEVIEW;
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	m_font = MAINFONT16;

	m_svlistbg = Image(this, "", "gui/mmbg.jpg", true, NULL, 1, 1, 1,0.2f, 0, 0, 1, 1);
	m_selected = NULL;
	m_scroll[1] = 0;
	m_selfile = NULL;

	m_vscroll = VScroll(this, "m_vscroll");
	m_savebut = Button(this, "save but", "gui/transp.png", (STRTABLE[STR_SAVE]), (STRTABLE[STR_SAVETOFILE]), m_font, BUST_LINEBASED, Resize_SV_SaveBut, Click_SV_Save, NULL, NULL, NULL, NULL, -1, NULL);
	m_delbut = Button(this, "del but", "gui/transp.png", (STRTABLE[STR_DELETE]), (STRTABLE[STR_DELTHESAVE]), m_font, BUST_LINEBASED, Resize_SV_DelBut, Click_SV_Del, NULL, NULL, NULL, NULL, -1, NULL);

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void SaveView::inev(InEv* ie)
{
	m_vscroll.inev(ie);
	m_savebut.inev(ie);
	m_delbut.inev(ie);
	m_curname.inev(ie);

	Player* py = &g_player[g_localP];

	m_scroll[1] = m_vscroll.m_scroll[1] * m_files.size()  * (1.0f);

	if(!ie->intercepted)
	{
		if(ie->type == INEV_MOUSEDOWN && ie->key == MOUSE_LEFT)
		{
			if(g_mouse.x >= m_pos[0] && g_mouse.x <= m_pos[2] && g_mouse.y >= m_listtop && g_mouse.y <= m_listbot)
			{
				m_ldown = true;
				int32_t sin = 0;
				BmpFont* f = &g_font[m_font];
				float scrollspace = m_listbot - m_listtop;
				int32_t viewable = scrollspace / f->gheight;
				float scroll = m_scroll[1];
				int32_t viewi = 0;

				for(std::list<RichText>::iterator sit=m_files.begin(); sit!=m_files.end(); sit++, sin++)
				{
					if(sin < scroll)
						continue;

					if(sin - scroll > viewable)
						break;

					if(g_mouse.y >= m_listtop + (sin-scroll)*f->gheight &&
						g_mouse.y <= m_listtop + (sin-scroll+1.0f)*f->gheight)
					{
						m_selfile = &*sit;
						m_curname.changevalue(m_selfile);
						ie->intercepted = true;
						break;
					}

					viewi++;
				}
			}
		}
		else if(ie->type == INEV_MOUSEUP && ie->key == MOUSE_LEFT)
		{
			if(m_ldown)
				ie->intercepted = true;
		}
	}

	if(ie->type == INEV_MOUSEMOVE)
	{
	}

	Win::inev(ie);
}

void SaveView::regen(unsigned char savemode)
{
	m_selfile = NULL;
	m_savemode = savemode;
	freech();
	m_files.clear();

	char datetime[64];
	sprintf(datetime, "%s.sav", FileDateTime().c_str());
	m_curname = EditBox(this, "cur name", RichText(datetime), MAINFONT16, Resize_SV_CurName, false, 64, NULL, NULL, -1);

	std::list<std::string> files;
	char fullpath[WF_MAX_PATH+1];
	FullWritePath(SAVEMODEPATH[savemode], fullpath);
	ListFiles(fullpath, files);

	for(std::list<std::string>::iterator fi=files.begin(); fi!=files.end(); fi++)
	{
		const char* fs = fi->c_str();
		m_files.push_back(RichText(UStr(fs)));
	}

	reframe();
}

void SaveView::frameupd()
{
	for(std::list<Widget*>::iterator i=m_subwidg.begin(); i!=m_subwidg.end(); i++)
		(*i)->frameupd();

	m_vscroll.frameupd();

	m_scroll[1] = m_vscroll.m_scroll[1] * m_files.size()  * (1.0f);
}

void SaveView::reframe()
{
	Win::reframe();

	BmpFont* f = &g_font[m_font];
	m_listtop = m_pos[1] + f->gheight;
	m_listbot = m_pos[3] - SLLISTBOT;

	m_vscroll.m_pos[0] = m_pos[2] - 16;
	m_vscroll.m_pos[1] = m_listtop;
	m_vscroll.m_pos[2] = m_pos[2];
	m_vscroll.m_pos[3] = m_listbot;
	float scrollable = f->gheight * m_files.size();
	float viewable = m_listbot - m_listtop;
	m_vscroll.m_domain = fmin(1, viewable / scrollable);
	m_vscroll.reframe();

	m_savebut.reframe();
	m_delbut.reframe();
	m_curname.reframe();
}

void SaveView::draw()
{
	Win::draw();

	//m_svlistbg.draw();

	int32_t sin = 0;
	BmpFont* f = &g_font[m_font];
	float scrollspace = m_listbot - m_listtop;
	int32_t viewable = scrollspace / f->gheight;
	float scroll = m_scroll[1];
	int32_t viewi = 0;

	for(std::list<RichText>::iterator sit=m_files.begin(); sit!=m_files.end(); sit++, sin++)
	{
		if(sin < scroll)
			continue;

		if(sin - scroll > viewable)
			break;

		if(&*sit == m_selfile)
		{
			UseS(SHADER_COLOR2D);
			Player* py = &g_player[g_localP];

			Shader* s = &g_shader[g_curS];
			glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
			glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);
			//glUniform4f(g_shader[SHADER_ORTHO].slot[SSLOT_COLOR], 0.2f, 1.0f, 0.2f, 0.6f);

			DrawSquare(0.2f, 1.0f, 0.2f, 0.6f, m_pos[0], m_listtop + (sin-scroll)*f->gheight, m_pos[2], m_listtop + (sin-scroll+1.0f)*f->gheight, m_crop);
			
			CHECKGLERROR();
			Ortho(g_width, g_height, 1, 1, 1, 1);
		}

		float namecolor[4] = {0.9f, 0.6f, 0.2f, 0.9f};
		DrawShadowedTextF(m_font, m_pos[0], (int32_t)(m_listtop + (sin-scroll)*f->gheight), m_pos[0], m_pos[1], m_pos[2], m_listbot, &*sit, namecolor);

		viewi++;
	}

	m_vscroll.draw();
	m_curname.draw();
	m_savebut.draw();
	m_delbut.draw();
}

void SaveView::drawover()
{
	Win::drawover();

	m_vscroll.drawover();
	m_curname.drawover();
	m_savebut.drawover();
	m_delbut.drawover();
}

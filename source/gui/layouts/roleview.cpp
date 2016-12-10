

#include "roleview.h"
#include "../../../sim/player.h"
#include "../../../econ/state.h"
#include "../../../econ/firm.h"

//TODO add ability to add, or not, close button to Win in constructor

void Resize_RoleItem(Widget* thisw)
{
	BmpFont* f = &g_font[thisw->m_font];
	int32_t row;
	char token[64];

	sscanf(thisw->m_name.c_str(), "%s %d", token, &row);

	Widget* parw = thisw->m_parent;
	
	if(strcmp(token, "num") == 0)
	{
		thisw->m_pos[0] = parw->m_scar[0] + 0;
		thisw->m_pos[2] = parw->m_scar[0] + 1000;
		thisw->m_pos[1] = parw->m_scar[1] + row * f->gheight;
		thisw->m_pos[3] = parw->m_scar[1] + (row+1) * f->gheight;
	}
	else if(strcmp(token, "roletext") == 0)
	{
		thisw->m_pos[0] = parw->m_scar[0] + 20;
		thisw->m_pos[2] = parw->m_scar[0] + 320;
		thisw->m_pos[1] = parw->m_scar[1] + row * f->gheight;
		thisw->m_pos[3] = parw->m_scar[1] + (row+1) * f->gheight;
	}
	else if(strcmp(token, "rolepick") == 0)
	{
		thisw->m_pos[0] = parw->m_scar[0] + 20;
		thisw->m_pos[2] = parw->m_scar[0] + 320;
		thisw->m_pos[1] = parw->m_scar[1] + row * f->gheight;
		thisw->m_pos[3] = parw->m_scar[1] + (row+1) * f->gheight;
	}
}

void Change_Role()
{
}

RoleView::RoleView(Widget* parent, const char* n, void (*reframef)(Widget* w)) : Win(parent, n, reframef)
{
	m_parent = parent;
	m_type = WIDGET_WINDOW;	//TODO not making any new widget types anymore
	m_name = n;
	reframefunc = reframef;
	m_ldown = false;
	m_font = MAINFONT16;

	for(int32_t i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];
		char wname[64];
		sprintf(wname, "num %d", i);
		char wtext[64];
		sprintf(wtext, "%d.", i+1);
		add(new Text(this, wname, RichText(wtext), MAINFONT16, Resize_RoleItem));
		
		sprintf(wname, "roletext %d", i);
		sprintf(wtext, "---");
		add(new Text(this, wname, RichText(wtext), MAINFONT16, Resize_RoleItem));
		Widget* lastw = *m_subwidg.rbegin();
		//lastw->hide();

		sprintf(wname, "rolepick %d", i);
		add(new DropList(this, wname, MAINFONT16, Resize_RoleItem, Change_Role));
		lastw = *m_subwidg.rbegin();
		//lastw->hide();

		for(int32_t si=0; si<STATES; si++)
		{
			char cname[64];
			sprintf(cname, "State %d", 1+si);
			RichText rname = RichText(cname);
			lastw->m_options.push_back(rname);
		}
		
		for(int32_t fi=0; fi<FIRMS; fi++)
		{
			char cname[64];
			sprintf(cname, "Firm %d", 1+fi);
			RichText rname = RichText(cname);
			lastw->m_options.push_back(rname);
		}
	}

	if(reframefunc)
		reframefunc(this);

	reframe();
}

void RoleView::regen()
{
	for(int32_t i=0; i<PLAYERS; i++)
	{
		Player* py = &g_player[i];
		char wname[64];
		
		sprintf(wname, "roletext %d", i);
		Widget* roletext = get(wname);
		roletext->hide();

		sprintf(wname, "rolepick %d", i);
		Widget* rolepick = get(wname);
		rolepick->hide();

		if(g_localP == i)
		{
			rolepick->show();

			int32_t choice = 0;

			if(py->insttype == INST_STATE)
			{
			}
			else if(py->insttype == INST_FIRM)
			{
				choice += STATES;
			}

			choice += py->instin;

			rolepick->m_selected = choice;
		}
		else
		{
			roletext->show();

			char wtext[64] = "";

			if(py->insttype == INST_STATE)
			{
				sprintf(wtext, "State %d", (int32_t)py->instin + 1);
			}
			else if(py->insttype == INST_FIRM)
			{
				sprintf(wtext, "Firm %d", (int32_t)py->instin + 1);
			}

			roletext->m_text = RichText(wtext);
		}
	}


}











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
#include "botpan.h"
#include "../../../platform.h"
#include "../viewportw.h"
#include "../../../sim/player.h"
#include "../../layouts/appviewport.h"

BotPan_init(BotPan* bp, Widget* parent, const char* n, void (*reframef)(Widget* w))
{
	ecbool *b;

	bp->parent = parent;
	bp->type = WIDGET_BOTTOMPANEL;
	strcpy(bp->name, n);
	bp->reframefunc = reframef;
	bp->ldown = ecfalse;

	for(b=bp->buto; b<bp->buto+9; ++b)
	{
		*b = ecfalse;
	}

	Widget_reframe((Widget*)bp);
}

void BotPan_reframe(BotPan* bp)	/* resized or moved */
{
	float left, top;
	int x, y, i;
	Button* b;

	Widget_reframe((Widget*)this);

	left = pos[2] - MINIMAP_SIZE - MINIMAP_OFF;
	top = pos[1] + 40 - MINIMAP_OFF;

	for(y=0; y<3; y++)
	{
		for(x=0; x<3; x++)
		{
			i = y*3 + x;

			b = but+i;

			b->pos[0] = left + MINIMAP_SIZE/3 * x;
			b->pos[1] = top + MINIMAP_SIZE/3 * y;
			b->pos[2] = left + MINIMAP_SIZE/3 * (x+1);
			b->pos[3] = top + MINIMAP_SIZE/3 * (y+1);
			Widget_reframe((Widget*)b);

			CenterLabel(b);
		}
	}
}

void BotPan_draw(BotPan* bp)
{
	Shader* s;
	int x, y, i;
	Button* b;
	Node *it;
	
	s = g_sh+g_curS;

	glUniform4f(s->slot[SSLOT_COLOR], 1, 1, 1, 1);

	for(y=0; y<3; y++)
	{
		for(x=0; x<3; x++)
		{
			i = y*3 + x;

			if(!*(buto+i))
				continue;

			b = but+i;
			Widget_draw((Widget*)b);
		}
	}
}

void BotPan_inev(BotPan* bp, InEv* ie)
{
	Button *b;
	ecbool *bo;

	for(bo=buto, b=but; bo<buto+9; ++bo,++b)
	{
		if(*bo)
		{
			Widget_inev((Widget*)b, ie);
		}
	}
}

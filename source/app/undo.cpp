

#include "undo.h"
#include "../render/sortb.h"
#include "../math/camera.h"
#include "../utils.h"
#include "../save/edmap.h"
#include "sesim.h"

std::list<UndoH> g_undoh;
int g_currundo = -1;	//the index which we will revert to when pressing undo next time
ecbool g_savedlatest = ecfalse;
int g_maxundo = 8;
//ecbool g_doubleredo = ecfalse;
//ecbool g_doubleundo = ecfalse;
//ecbool g_oncurrh = ecfalse;

//#define UNDO_DEBUG

UndoH::~UndoH()
{
#if 0
	Log("~UndoH()");
#endif
}

//write undo history
void WriteH(UndoH* towrite)
{
	//Important. For some reason there's a memory leak here
	//if we don't call clear(). Bug in std::list<>?
	towrite->brushes.clear();
	towrite->brushes = g_edmap.brush;
	towrite->modelholders.clear();
	towrite->modelholders = g_modelholder;

	for(int k=0; k<4; k++)
		towrite->tiletexs[k] = g_tiletexs[k];

#ifdef UNDO_DEBUG
	Log("save h "<<towrite->brushes.size()<<" brushes");
	
#endif
}

void LinkPrevUndo(UndoH* tosave)
{
	UndoH h;

	if(tosave != NULL)
		h = *tosave;
	else
	{
		WriteH(&h);
	}

#ifdef UNDO_DEBUG
	Log("linkpr gcurrun="<<g_currundo<<std::endl;
	
#endif

	g_savedlatest = ecfalse;
	//g_doubleredo = ecfalse;
	//g_doubleundo = ectrue;

	int j=0;
	std::list<UndoH>::iterator i=g_undoh.begin();
	while(i!=g_undoh.end())
	{
#ifdef UNDO_DEBUG
		Log("erase? "<<j<<std::endl;
		
#endif

		if(j > g_currundo)
		{
#ifdef UNDO_DEBUG
			Log("erase. "<<j<<std::endl;
			
#endif
			i = g_undoh.erase(i);
			j++;
			continue;
		}

		i++;
		j++;
	}

	g_currundo++;
	//if(g_currundo >= g_maxundo)
	//	g_currundo = g_maxundo-1;
	if(g_currundo > g_maxundo)
		g_currundo = g_maxundo;

#ifdef UNDO_DEBUG
	Log("linkpr gcurrun="<<g_currundo<<std::endl;
	
#endif

	g_undoh.push_back(h);

#ifdef UNDO_DEBUG
	Log("linkpr undoh.size="<<g_undoh.size()<<std::endl;
	
#endif

	int overl = (int)g_undoh.size() - g_maxundo;
	if(overl > 0)
	{
		j=0;
		i=g_undoh.begin();
		while(i!=g_undoh.end() && overl > j)
		{
			i = g_undoh.erase(i);
			j++;
		}
	}

#ifdef UNDO_DEBUG
	Log("linkpr undoh.size2="<<g_undoh.size()<<std::endl;
	
#endif
}

void LinkLatestUndo()
{
#ifdef UNDO_DEBUG
	Log("linklate "<<g_currundo<<" == "<<((int)g_undoh.size()-1)<<std::endl;
	
#endif

	if(g_currundo >= (int)g_undoh.size()-1 && !g_savedlatest)	//only save if we're at the latest undo
	{
		LinkPrevUndo();
		g_currundo--;
		g_savedlatest = ectrue;
		//g_doubleredo = ectrue;
		//g_doubleundo = ecfalse;
	}
}

void Undo()
{
	//g_doubleredo = ectrue;

#ifdef UNDO_DEBUG
	Log("undo? g_curu="<<g_currundo<<std::endl;
	
#endif

	if(g_currundo <= -1)
	{
		//g_doubleredo = ecfalse;
		return;
	}

#ifdef UNDO_DEBUG
	Log("undo from1 "<<g_currundo<<" of "<<g_undoh.size()<<std::endl;
	
#endif

	LinkLatestUndo();
	/*
	if(g_doubleundo)
	{
		//g_doubleundo = ecfalse;
		g_currundo --;

		if(g_currundo <= -1)
		{
			//g_doubleredo = ecfalse;
			return;
		}
	}*/

#ifdef UNDO_DEBUG
	Log("undoh.soze="<<g_undoh.size()<<std::endl;
	
#endif

	int j=0;
	for(std::list<UndoH>::iterator i=g_undoh.begin(); i!=g_undoh.end(); i++, j++)
	{
#ifdef UNDO_DEBUG
		Log("undoh #"<<j<<std::endl;
		
#endif

		if(j == g_currundo)
		{
			UndoH* h = &*i;
			g_edmap.brush = h->brushes;
			g_modelholder = h->modelholders;

			for(int k=0; k<4; k++)
				g_tiletexs[k] = h->tiletexs[k];

#ifdef UNDO_DEBUG
			Log("undid now "<<g_edmap.brush.size()<<" brushes");
			
#endif

			break;
		}
	}

#ifdef UNDO_DEBUG
	Log("undo from2 "<<g_currundo<<" of "<<g_undoh.size()<<std::endl;
	
#endif

	g_currundo--;
	if(g_currundo < 0)
	{
		g_currundo = 0;
		//return;
	}

	//if(g_currundo <= 0)
	//	g_doubleredo = ecfalse;

	g_sel1b = NULL;
	g_selB.clear();
	g_dragW = -1;
	g_dragS = -1;
	g_dragV = -1;
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);

#ifdef UNDO_DEBUG
	Log("undo to "<<g_currundo<<" of "<<g_undoh.size()<<std::endl;
	
#endif

	//g_oncurrh = ectrue;
	//g_doubleredo = ectrue;
	//g_doubleundo = ecfalse;
}

void Redo()
{
	//g_doubleundo = ectrue;
	g_currundo++;	//moves to current state
	/*
	if(g_doubleredo)
	{
		g_currundo++;	//moves to next state
	}*/

	if(g_currundo >= g_undoh.size())
	{
		g_currundo = g_undoh.size()-1;
	//if(g_currundo > g_undoh.size())
	//{
	//	g_currundo = g_undoh.size();
	//if(g_currundo > g_undoh.size()-2)
	//{
	//	g_currundo = g_undoh.size()-2;
		return;
	}

	int j=0;
	for(std::list<UndoH>::iterator i=g_undoh.begin(); i!=g_undoh.end(); i++, j++)
	{
		if(j == g_currundo)
		{
			UndoH* h = &*i;
			g_edmap.brush = h->brushes;
			g_modelholder = h->modelholders;

#ifdef UNDO_DEBUG
			Log("redid to "<<g_edmap.brush.size()<<" brushes");
			
#endif
			break;
		}
	}
	/*
	if(g_doubleredo)
	{
		//g_doubleredo = ecfalse;
		g_currundo--;	//move back to new current state
	}*/

	g_sel1b = NULL;
	g_selB.clear();
	g_dragW = -1;
	g_dragS = -1;
	g_dragV = -1;
	SortEdB(&g_edmap, g_cam.view, g_cam.pos);

#ifdef UNDO_DEBUG
	Log("redo to "<<g_currundo<<" of "<<g_undoh.size()<<std::endl;
	
#endif

	if(g_currundo > g_undoh.size()-2)
	{
		g_currundo = g_undoh.size()-2;
	}

	//g_doubleredo = ecfalse;
	//g_doubleundo = ectrue;
}

void ClearUndo()
{
#ifdef UNDO_DEBUG
	Log("clear undo");
	
#endif

	g_currundo = -1;
	g_undoh.clear();
	g_savedlatest = ecfalse;
	//g_doubleundo = ectrue;
}

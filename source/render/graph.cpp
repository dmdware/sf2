












#include "graph.h"
#include "../sim/simflow.h"
#include "../sim/unit.h"
#include "../sim/building.h"
#include "../sim/conduit.h"
#include "../sim/resources.h"
#include "../sim/simdef.h"
#include "../gui/widgets/spez/pygraphs.h"
#include "../gui/widgets/spez/blgraphs.h"
#include "../gui/widgets/spez/gengraphs.h"
#include "../language.h"
#include "../gui/layouts/chattext.h"

Graph g_graph[GRAPHS];
const RichText* GRAPHNAME[GRAPHS] =
{&STRTABLE[STR_AVPS],
&STRTABLE[STR_AVPE],
&STRTABLE[STR_TOTPS],
&STRTABLE[STR_TOTPE]};

Graph g_protecg[PLAYERS];

void DrawGraph(Graph* g, float left, float top, float right, float bottom, float highest, const float* color)
{
	EndS();
	UseS(SHADER_COLOR2D);
	Shader* s = g_sh+g_curS;

	//float highest = 0;
	float lowest = g->lowest;

#if 0
	for(std::list<Widget*>::iterator pit=g->points.begin(); pit!=g->points.end(); pit++)
	{
		if(*pit < lowest)
			lowest = *pit;

		if(*pit > highest)
			highest = *pit;
	}
#else
	//if(g->highest > highest)
	//	highest = g->highest;
#endif

	//float xadv = (right-left) / (float)(g->points.size()-1);
	//float yadv = (bottom-top) / (highest-lowest);

#if 0
	std::vector<Vec3f> ps;
	ps.reserve( g->points.size() );
	
	int x = 0;
	for(std::list<Widget*>::iterator pit=g->points.begin(); pit!=g->points.end(); pit++)
	{
		ps.push_back( Vec3f( left + xadv*x, top + ((float)highest - (float)*pit)*yadv, 0.0f ) );
		x++;
	}
#endif

	Py* py = &g_py[g_localP];
	if(!color)
		glUniform4f(s->slot[SSLOT_COLOR], 0.1f, 0.9f, 0.1f, 1.0f);
	else
		glUniform4f(s->slot[SSLOT_COLOR], color[0], color[1], color[2], 1.0f);
	glUniform1f(s->slot[SSLOT_WIDTH], (float)g_width);
	glUniform1f(s->slot[SSLOT_HEIGHT], (float)g_height);

	//if(ps.size() <= 0)
	if(g->ps.size() <= 0)
		return;

#if 0
	char m[123];
	sprintf(m, "h%f", highest);
	RichText rm = RichText(m);
	AddNotif(&rm);
	Log("%s", m);
#endif

#ifdef PLATFORM_GL14
	//glVertexPointer(3, GL_FLOAT, sizeof(float)*0, &ps[0]);
	glVertexPointer(3, GL_FLOAT, sizeof(float)*0, &g->ps[0]);
#endif
	
#ifdef PLATFORM_GLES20
	//glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &ps[0]);
	glVertexAttribPointer(s->slot[SSLOT_POSITION], 3, GL_FLOAT, GL_FALSE, sizeof(float)*0, &g->ps[0]);
#endif
	//glVertexAttribPointer(s->slot[SSLOT_TEXCOORD0], 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, &vertices[2]);
	//glTexCoordPointer(2, GL_FLOAT, sizeof(float)*0, &vertices[2]);
	//glDrawArrays(GL_LINE_STRIP, 0, ps.size());
	glDrawArrays(GL_LINE_STRIP, 0, g->ps.size());
	CHECKGLERROR();

	EndS();	//corpd fix

	CHECKGLERROR();
	Ortho(g_width, g_height, 1.0f, 1.0f, 1.0f, 1.0f);
}

//tally graph data
void Tally()
{
	RecStats();
	RecPyStats();

	unsigned __int64 froff = g_simframe % CYCLE_FRAMES;

	//do this every 1 frame in CYCLE_FRAMES (once a minute)
	if(froff != 0)
		return;

	//Tally protectionism stats
	for(int i=0; i<PLAYERS; i++)
	{
		g_protecg[i].points.push_back((float)g_py[i].global[RES_DOLLARS]);
	}

	{
		float graphbounds[4];
		graphbounds[0] = (float)g_width/2;
		graphbounds[1] = (float)g_height - 60 - 130;
		graphbounds[2] = (float)g_width - 30;
		graphbounds[3] = (float)g_height - 60;
		float highest=0;
		for(int i=0; i<PLAYERS; i++)
			//for(int i=1; i<(FIRMSPERSTATE+1)*4; i+=(FIRMSPERSTATE+1))
		{
#if 0
			for(std::list<Widget*>::iterator pit=g_protecg[i].points.begin(); pit!=g_protecg[i].points.end(); pit++)
			{
				highest = fmax(highest,  *pit);
				highest = fmax(highest,  -*pit);
			}
#else
			highest = fmax(g_protecg[i].highest, highest);
#endif
		}
		
		for(int i=0; i<PLAYERS; i++)
		{
			g_protecg[i].up(graphbounds[0], graphbounds[1], graphbounds[2], graphbounds[3], highest);
		}
	}

	float graphbounds[4];
	graphbounds[0] = (float)g_width/2;
	graphbounds[1] = (float)30;
	graphbounds[2] = (float)g_width - 30;
	graphbounds[3] = (float)30 + 130;

	//Tally average personal satiety
	{
		Graph* g = &g_graph[GR_AVGFOD];
		float p = 0;
		int pcnt = 0;

		for(int i=0; i<MOVERS; i++)
		{
			Mv* mv = &g_mv[i];

			if(!mv->on)
				continue;

			if(mv->type != MV_LABOURER)
				continue;

			p = ( p * (float)pcnt + (float)mv->belongings[RES_RETFOOD] ) / (float)(pcnt+1);
			pcnt++;
		}

		if(g->points.size() <= 0)
		{
			g->startframe = g_simframe;
			g->cycles = 0;
		}
		else
			g->cycles++;

		g->points.push_back(p);

		g->up(graphbounds[0], graphbounds[1], graphbounds[2], graphbounds[3], 0);
	}

	//Tally average personal funds
	{
		Graph* g = &g_graph[GR_AVGFUN];
		float p = 0;
		int pcnt = 0;

		for(int i=0; i<MOVERS; i++)
		{
			Mv* mv = &g_mv[i];

			if(!mv->on)
				continue;

			if(mv->type != MV_LABOURER)
				continue;

			p = ( p * (float)pcnt + (float)mv->belongings[RES_DOLLARS] ) / (float)(pcnt+1);
			pcnt++;
		}

		if(g->points.size() <= 0)
		{
			g->startframe = g_simframe;
			g->cycles = 0;
		}
		else
			g->cycles++;

		g->points.push_back(p);

		g->up(graphbounds[0], graphbounds[1], graphbounds[2], graphbounds[3], 0);
	}

	//Tally total personal satiety
	{
		Graph* g = &g_graph[GR_TOTFOD];
		unsigned __int64 p = 0;
		int pcnt = 0;

		for(int i=0; i<MOVERS; i++)
		{
			Mv* mv = &g_mv[i];

			if(!mv->on)
				continue;

			if(mv->type != MV_LABOURER)
				continue;

			p += mv->belongings[RES_RETFOOD];
			pcnt++;
		}

		if(g->points.size() <= 0)
		{
			g->startframe = g_simframe;
			g->cycles = 0;
		}
		else
			g->cycles++;

		g->points.push_back((float)p);

		g->up(graphbounds[0], graphbounds[1], graphbounds[2], graphbounds[3], 0);
	}
	
	//Tally total personal funds
	{
		Graph* g = &g_graph[GR_TOTFUN];
		unsigned __int64 p = 0;
		int pcnt = 0;

		for(int i=0; i<MOVERS; i++)
		{
			Mv* mv = &g_mv[i];

			if(!mv->on)
				continue;

			if(mv->type != MV_LABOURER)
				continue;

			p += mv->belongings[RES_DOLLARS];
			pcnt++;
		}

		if(g->points.size() <= 0)
		{
			g->startframe = g_simframe;
			g->cycles = 0;
		}
		else
			g->cycles++;

		g->points.push_back((float)p);

		g->up(graphbounds[0], graphbounds[1], graphbounds[2], graphbounds[3], 0);
	}
}

void FreeGraphs()
{
	for(int i=0; i<GRAPHS; i++)
	{
		Graph* g = &g_graph[i];
		g->points.clear();
		g->ps.clear();
		g->highest = g->lowest = 0;
		g->cycles = 0;
		g->startframe = 0;
	}

	//2016/05/03	clear p g
	for(int i=0; i<PLAYERS; ++i)
	{
		Graph* g = &g_protecg[i];
		g->points.clear();
		g->ps.clear();
		g->highest = g->lowest = 0;
		g->cycles = 0;
		g->startframe = 0;
	}
}
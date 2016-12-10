













#ifndef GRAPH_H
#define GRAPH_H

#include "../platform.h"
#include "../sim/player.h"
#include "../math/vec3f.h"

struct Graph
{
public:
	std::list<float> points;
	unsigned __int64 startframe;
	unsigned int cycles;
	std::vector<Vec3f> ps;
	float lowest;
	float highest;

	Graph()
	{
		startframe = 0;
		cycles = 0;
	}

	void up(float left, float top, float right, float bottom, float highestin)
	{
		lowest = 0;
		highest = highestin;

		for(std::list<float>::iterator pit=points.begin(); pit!=points.end(); pit++)
		{
			if(*pit < lowest)
				lowest = *pit;

			if(*pit > highest)
				highest = *pit;
		}

		float xadv = (right-left) / (float)(points.size()-1);
		float yadv = (bottom-top) / (highest-lowest);

		ps.clear();
		ps.reserve( points.size() );
	
		int x = 0;
		for(std::list<float>::iterator pit=points.begin(); pit!=points.end(); pit++)
		{
			//ps.push_back( Vec3f( left + xadv*x, top + ((float)highest - (float)*pit)*yadv, 0.0f ) );
			ps.push_back( Vec3f( left + xadv*x, top + ((float)highest - (float)*pit)*yadv, 0.0f ) );
			x++;
		}
	}
};

#define GR_AVGFOD		0	//average personal food
#define GR_AVGFUN		1	//average personal funds
#define GR_TOTFOD		2	//total personal food
#define GR_TOTFUN		3	//total personal funds
#define GRAPHS			4

extern const RichText* GRAPHNAME[GRAPHS];

extern Graph g_graph[GRAPHS];

extern Graph g_protecg[PLAYERS];

void Tally();
void FreeGraphs();
void DrawGraph(Graph* g, float left, float top, float right, float bottom, float highest=0, const float* color=NULL);

#endif
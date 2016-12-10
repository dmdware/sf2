










#ifndef EDITORGUI_H
#define EDITORGUI_H

#include "../../platform.h"
#include "../../math/vec2i.h"

#define TOOLCAT_NEWMAP			0
#define TOOLCAT_ELEVATION		1
#define TOOLCAT_TERRAIN			2
#define TOOLCAT_FOLIAGE			3
#define TOOLCAT_UNITS			4
#define TOOLCAT_BUILDINGS		5
#define TOOLCAT_CONDUITS		6
#define TOOLCAT_RESOURCES		7
#define TOOLCAT_TRIGGERS		8
#define TOOLCAT_BORDERS			9
#define TOOLCAT_PLAYERS			10
#define TOOLCAT_SCRIPTS			11

#define TOOLACT_PLACE			0
#define TOOLACT_DELETE			1

#define ELEVACT_RAISE           0
#define ELEVACT_LOWER           1
#define ELEVACT_SPREAD          2


void Resize_LeftPanel(Widget* w);
void FillEd();
int GetEdUType();
int GetEdBlType();
int GetEdCdType();
int GetEdTool();
int GetEdAct();
int GetElevAct();
int GetPlaceOwner();
int GetPlaceOwner2();
void Change_ToolCat();

void LowerTerr(Vec2i from, Vec2i to);
void RaiseTerr(Vec2i from, Vec2i to);
void SpreadTerr(Vec2i from, Vec2i to);
void EdPlaceBords();

#endif

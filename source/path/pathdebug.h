










#ifndef PATHDEBUG_H
#define PATHDEBUG_H

class Unit;

extern Unit* g_pathunit;
extern std::vector<Vec3f> g_gridvecs;

void DrawSteps();
void DrawGrid();
void DrawUnitSquares();
void DrawPaths();
void DrawVelocities();
void LogPathDebug();

#endif

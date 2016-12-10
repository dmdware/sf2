

#include "../bsp/brush.h"
#include "../sim/map.h"
#include "../platform.h"
#include "../save/modelholder.h"
#include "sesim.h"

struct UndoH	//undo history
{
public:
	std::list<Brush> brushes;
	std::list<ModelHolder> modelholders;
	unsigned int tiletexs[TEX_TYPES];

	UndoH(){}
	~UndoH();
};

//#define g_maxundo		16
extern int g_maxundo;

void LinkPrevUndo(UndoH* tosave=NULL);	//call this BEFORE the change is made
void LinkLatestUndo();	//called by undo itself
void WriteH(UndoH* writeto);
void Undo();
void Redo();
void ClearUndo();

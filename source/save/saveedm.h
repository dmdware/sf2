


#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/polygon.h"
#include "../math/triangle.h"
#include "../bsp/brush.h"

struct Map
{
public:
	int nbrush;
	Brush* brush;
	std::list<int> transpbrush;
	std::list<int> opaquebrush;
	std::list<int> skybrush;

	Map();
	~Map();
	void destroy();
};

extern Map g_map;

void DrawMap(Map* map);
void DrawMap2(Map* map);

#define EDMAP_VERSION		2.0f

#define TAG_EDMAP		{'D', 'M', 'D', 'S', 'P'}	//DMD sprite project

struct EdMap;
struct TexRef;
struct Brush;

struct Map;
struct CutBrush;
struct Brush;

void ReadBrush(FILE* fp, TexRef* texrefs, Brush* b);
void SaveBrush(FILE* fp, int* texrefs, Brush* b);
void SaveEdMap(const char* fullpath, EdMap* map);
ecbool LoadEdMap(const char* fullpath, EdMap* map);
void FreeEdMap(EdMap* map);

#define TAG_MAP		{'D', 'M', 'D', 'M', 'C'}
//#define MAP_VERSION		1.0f

void SaveTexs(FILE* fp, int* texrefs, std::list<Brush>& brushes);
void SaveMap(const char* fullpath, std::list<Brush>& brushes);
ecbool LoadMap(const char* fullpath, Map* map);












#include "../platform.h"
#include "../math/vec3f.h"
#include "../math/vec3i.h"
#include "../math/vec2i.h"

struct OrderMarker
{
public:
	Vec2i pos;
	int tick;
	float radius;

	OrderMarker(Vec2i p, int t, float r)
	{
		pos = p;
		tick = t;
		radius = r;
	}
};

extern std::list<OrderMarker> g_order;

#define ORDER_EXPIRE		2000

struct MoveOrderPacket;

void DrawOrders(Matrix* projection, Matrix* modelmat, Matrix* viewmat);
void Order(int mousex, int mousey, int viewwidth, int viewheight);
void MoveOrder(MoveOrderPacket* mop);

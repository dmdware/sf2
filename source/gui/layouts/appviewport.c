












#include "../../render/shader.h"
#include "../../gui/gui.h"
#include "../../math/3dmath.h"
#include "../../window.h"
#include "../../platform.h"
#include "../../gui/font.h"
#include "../../math/camera.h"
#include "../../math/matrix.h"
#include "../../render/heightmap.h"
#include "../../math/vec4f.h"
#include "../../math/brush.h"
#include "../../math/frustum.h"
#include "../../sim/simdef.h"
#include "appviewport.h"
#include "../../math/hmapmath.h"
#include "../../render/water.h"
#include "../../save/savemap.h"
#include "../../gui/widgets/spez/botpan.h"
#include "../../sim/bltype.h"
#include "../../sim/mvtype.h"
#include "../../sim/player.h"
#include "../../debug.h"
#include "../../sim/conduit.h"

void VpType_init(VpType* vt, Vec3f offset, Vec3f up, const char* label, ecbool axial)
{
	vt->offset = offset;
	vt->up = up;
	strcpy(vt->label, label);
	vt->axial = axial;
}


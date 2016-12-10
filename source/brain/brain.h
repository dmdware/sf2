

#ifndef BRAIN_H
#define BRAIN_H

#include "../algo/localmap.h"


#define OPCODE_PLACE_BL
#define OPCODE_PLACE_CD
#define OPCODE_SAY
#define OPCODE_PLACE_MV
#define OPCODE_BL_ON
#define OPCODE_BL_OWNER
#define OPCODE_BL_FIN
#define OPCODE_BL_POS
#define OPCODE_TILE_TYPE
#define OPCODE_TILE_ID
#define OPCODE_BL_RES
#define OPCODE_CD_RES
#define OPCODE_BLTYPE_COST
#define OPCODE_CDTYPE_COST
#define OPCODE_RES_ISCAP
#define OPCODE_RES_VALID
#define OPCODE_RES_CDTYPE
#define OPCODE_CDTYPE_REQSRC
#define OPCODE_BL_CDNETW
#define OPCODE_MAPW
#define OPCODE_FEEDTO

#define OPCODE_THINK_BEG

#define OPCODE_ACT_BEG

//unsigned __int128
//__m128
// _umul128

#define BRAIN_BL_HOUSE1		0

#define BRAINCYCLES		1024

void TryHarder();
void TeachSeq(HASHINT in, HASHINT out, HASHINT p1);

extern ecbool g_braindone;

#endif
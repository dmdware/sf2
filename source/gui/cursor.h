










#ifndef CURSOR_H
#define CURSOR_H

#include "../render/sprite.h"

//cursor states
#define CU_NONE		0	//cursor off?
#define CU_DEFAULT	1
#define CU_MOVE		2	//move window
#define CU_RESZL	3	//resize width (horizontal) from left side
#define CU_RESZR	4	//resize width (horizontal) from right side
#define CU_RESZT	5	//resize height (vertical) from top side
#define CU_RESZB	6	//resize height (vertical) from bottom side
#define CU_RESZTL	7	//resize top left corner
#define CU_RESZTR	8	//resize top right corner
#define CU_RESZBL	9	//resize bottom left corner
#define CU_RESZBR	10	//resize bottom right corner
#define CU_WAIT		11	//shows a hourglass?
#define CU_DRAG		12	//drag some object between widgets?
#define CU_STATES	13

extern unsigned int g_cursor[CU_STATES];


#endif


#ifndef PHYSICS_H
#define PHYSICS_H


#define EPSILON			0.03125f //0.1f	//1.2f
//#define EPSILON			0.1f	//1.2f
#define	CLOSE_EPSILON	0.1f
#define FRICTION		1.5f
#define INVFRICTION		(1.0f/FRICTION)

//cm/sec^2 =  ?
//sec = 30 frame
//98.1 cm / sec ^ 2 = 98.1 cm / (30 frame) ^ 2
// = 0.109 cm/frame^2
#define GRAVITY		(10.309f/20)	// cm/frame^2

void Physics();

#endif
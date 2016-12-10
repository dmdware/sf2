








#include "connectable.h"

int ConnType(ecbool n, ecbool e, ecbool s, ecbool w)
{
	if(n && e && s && w)
		return CONNECTION_NORTHEASTSOUTHWEST;
	else if(n && e && s && !w)
		return CONNECTION_NORTHEASTSOUTH;
	else if(n && e && !s && w)
		return CONNECTION_NORTHEASTWEST;
	else if(n && e && !s && !w)
		return CONNECTION_NORTHEAST;
	else if(n && !e && s && w)
		return CONNECTION_NORTHSOUTHWEST;
	else if(n && !e && s && !w)
		return CONNECTION_NORTHSOUTH;
	else if(n && !e && !s && w)
		return CONNECTION_NORTHWEST;
	else if(n && !e && !s && !w)
		return CONNECTION_NORTH;
	else if(!n && e && s && w)
		return CONNECTION_EASTSOUTHWEST;
	else if(!n && e && s && !w)
		return CONNECTION_EASTSOUTH;
	else if(!n && e && !s && w)
		return CONNECTION_EASTWEST;
	else if(!n && e && !s && !w)
		return CONNECTION_EAST;
	else if(!n && !e && s && w)
		return CONNECTION_SOUTHWEST;
	else if(!n && !e && s && !w)
		return CONNECTION_SOUTH;
	else if(!n && !e && !s && w)
		return CONNECTION_WEST;
	else
		return CONNECTION_NOCONNECTION;

	/*
	 int r = rand3()%4;

	 if(r == 0)
	 return CONNECTION_NORTH;
	 else if(r == 1)
	 return CONNECTION_EAST;
	 else if(r == 2)
	 return CONNECTION_SOUTH;
	 else
	 return CONNECTION_WEST;*/
}


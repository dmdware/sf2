












#include "utility.h"
#include "../path/pathnode.h"


// food/housing/physical res utility evaluation
int PhUtil(int price, int cmdist)
{
	cmdist /= INV_DIST_EFFECT;
	price /= INV_WAGE_EFFECT;

	if(price <= 0)
		return MAX_UTIL;

	if(cmdist <= 0)
		return MAX_UTIL;
	
	return MAX_UTIL / price / (cmdist);
	//return MAX_UTIL / price / (cmdist / PATHNODE_SIZE);
	//return MAX_UTIL / price / (cmdist * PATHNODE_SIZE);
	//return MAX_UTIL - (price * cmdist);
}

// electricity utility evaluation
int GlUtil(int price)
{
	price /= INV_WAGE_EFFECT;

	if(price <= 0)
		return MAX_UTIL;
	
	return MAX_UTIL / price;
	//return MAX_UTIL - price;
}

// inverse phys utility - solve for distance based on utility and price
int InvPhUtilD(int util, int price)
{
	//util = 100000000 / price / (cmdist/PATHNODE_SIZE);
	//util / 100000000 = 1 / price / (cmdist//PATHNODE_SIZE);
	//util * price / 100000000 = 1 / (cmdist//PATHNODE_SIZE);
	//100000000 / (util * price) = (cmdist//PATHNODE_SIZE);
	
	price /= INV_WAGE_EFFECT;

	if(util <= 0)
		return MAX_UTIL;
	
	//if(price / PATHNODE_SIZE <= 0)
	if(price <= 0)
		return MAX_UTIL;
		//return -(util - MAX_UTIL);

	//util = MAX_UTIL - (price * cmdist)
	//util - MAX_UTIL = - (price * cmdist)
	//cmdist = - (util - MAX_UTIL)/price
	
	//return MAX_UTIL / (util * price);
	return MAX_UTIL / (util * price) * INV_DIST_EFFECT;
	//return MAX_UTIL / (util * price / PATHNODE_SIZE);
	//return MAX_UTIL / (util * price * PATHNODE_SIZE);
	//return -(util - MAX_UTIL)/price;
}

// inverse phys utility - solve for price based on utility and distance
int InvPhUtilP(int util, int cmdist)
{
	cmdist /= INV_DIST_EFFECT;

	if(util <= 0)
		return MAX_UTIL;
	
	//if(cmdist/PATHNODE_SIZE <= 0)
	if(cmdist <= 0)
		return MAX_UTIL;
		//return -(util - MAX_UTIL);
	
	return MAX_UTIL / (util * (cmdist)) * INV_WAGE_EFFECT;
	//return MAX_UTIL / (util * (cmdist/PATHNODE_SIZE));
	//return MAX_UTIL / (util * (cmdist*PATHNODE_SIZE));
	//return -(util - MAX_UTIL)/cmdist;
}

int InvGlUtilP(int util)
{
	if(util <= 0)
		return MAX_UTIL;
	
	return MAX_UTIL / util;
	//return MAX_UTIL - util;
}


int InvJobUtilP(int util, int cmdist, int workdelay)
{
	cmdist /= INV_DIST_EFFECT;
	
	if(util * cmdist <= 0)
		return MAX_UTIL * INV_WAGE_EFFECT;

	return iceil(MAX_UTIL, util * cmdist) * INV_WAGE_EFFECT;
}

int JobUtil(int wage, int cmdist, int workdelay)
{
	wage /= INV_WAGE_EFFECT;

	cmdist /= INV_DIST_EFFECT;

	if(wage <= 0)
		return 0;
	
	//if(cmdist/PATHNODE_SIZE <= 0)
	if(cmdist <= 0)
		return MAX_UTIL;

	if(workdelay <= 0)
		return MAX_UTIL;
	
	return MAX_UTIL / (cmdist) /* / workdelay */ * wage;
	//return MAX_UTIL / (cmdist/PATHNODE_SIZE) / workdelay * wage;
	//return MAX_UTIL / (cmdist*PATHNODE_SIZE) / workdelay * wage;
	//return MAX_UTIL - (cmdist * workdelay) / wage;
}
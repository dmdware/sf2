













#ifndef CONDITION_H
#define CONDITION_H

#include "../platform.h"

#define CONDITION_NONE			0
#define CONDITION_ONLOADMAP		1
#define CONDITION_ONPLACEB		2
#define CONDITION_ONBUILDU		3
#define CONDITION_ONPLACEOTHERB	4
#define CONDITION_ONBUILDOTHERU	5
#define CONDITION_PRICESET		6
#define CONDITION_UNITCOUNT		7
#define CONDITION_CAPTUREB		8
#define CONDITION_MESSAGECLOSED	9
#define CONDITION_ROADINACCESSABILITY	10
#define CONDITION_ALLROADACCESSIBLE		11
#define CONDITION_ONFINISHEDB	12
#define CONDITION_ALLCONSTRUCTIONFINI	13

#define CONDITION_LEN		31	//not counting null terminator

struct Condition
{
public:
	char name[CONDITION_LEN+1];
	int type;
	ecbool met;
	int target;
	float fval;
	int count;
	int owner;
    
	Condition();
};


void OnPriceSet(int targ, float val);
void OnLoadMap();
void OnRoadInacc();
void OnAllRoadAc();
void OnCaptureB(int target);
void OnBuildU(int target);
void OnPlaceB(int target);
void OnCloseMessage();
void OnFinishedB(int target);
ecbool ConditionMet(int type, int target, float fval, int count, int owner);

void GMessage(const char* text);
void GMessageG(const char* tex, int w, int h);
void Click_CloseMessage();
void ReGUICondition();
void ReGUITrigger();
void ReGUIScript();
void Click_CloseUnique();
ecbool UniqueCondition(const char* name);


#endif
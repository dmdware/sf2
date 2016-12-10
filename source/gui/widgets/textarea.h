










#ifndef TEXTAREA_H
#define TEXTAREA_H

#include "../widget.h"

struct TextArea : public Widget
{
public:
	int highl[2];	// highlighted (selected) text
	UStr compos;	//composition for unicode text

	TextArea(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), float r=1, float g=1, float b=1, float a=1, void (*change)()=NULL);

	void draw();
	int rowsshown();
	int square();

	float topratio()
	{
		return scroll[1] / (float)lines;
	}

	float bottomratio()
	{
		return (scroll[1]+rowsshown()) / (float)lines;
	}

	float scrollspace();
	void changevalue(const char* newv);
	ecbool delnext();
	ecbool delprev();
	void copyval();
	void pasteval();
	void selectall();
	//void placestr(const char* str);
	void placestr(const RichText* str);
	void placechar(unsigned int k);
	void inev(InEv* ie);
	void hide();
	void gainfocus();
	void losefocus();
};

#endif

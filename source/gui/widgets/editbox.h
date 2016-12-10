










#ifndef EDITBOX_H
#define EDITBOX_H

#include "../widget.h"

struct EditBox : public Widget
{
public:
	int highl[2];	// highlighted (selected) text
	UStr compos;	//composition for unicode text
	void (*submitfunc)();
	void (*changefunc3)(unsigned int key, unsigned int scancode, ecbool down, int parm);

	EditBox();
	EditBox(Widget* parent, const char* n, const RichText t, int f, void (*reframef)(Widget* w), ecbool pw, int maxl, void (*change3)(unsigned int key, unsigned int scancode, ecbool down, int parm), void (*submitf)(), int parm);

	void draw();
	RichText drawvalue();
	void frameupd();
	void inev(InEv* ie);
	void placestr(const RichText* str);
	void changevalue(const RichText* str);
	ecbool delnext();
	ecbool delprev();
	void copyval();
	void pasteval();
	void selectall();
	void hide();
	void gainfocus();
	void losefocus();
};

#endif

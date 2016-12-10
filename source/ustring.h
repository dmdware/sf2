











#ifndef USTRING_H
#define USTRING_H

#include "platform.h"

//#define USTR_DEBUG

struct UStr
{
public:
	unsigned int length;	//length doesn't count null-terminator
	unsigned int* data;

	UStr();
	~UStr();
	UStr(const UStr& original);
	UStr(const char* cstr);
	UStr(unsigned int k);
	UStr(unsigned int* k);
	UStr& operator=(const UStr &original);
	UStr operator+(const UStr &other);
	UStr substr(int start, int len) const;
	int firstof(UStr find) const;
	std::string rawstr() const;
};

#endif

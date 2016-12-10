











#include "platform.h"
#include "ustring.h"
#include "utils.h"
#include "sys/unicode.h"

UStr::UStr()
{
	data = new unsigned int[1];
	data[0] = 0;
	length = 0;
}

UStr::~UStr()
{
#ifdef USTR_DEBUG
	Log("delete UStr...";
	

	for(int i=0; i<length; i++)
	{
		Log((char)data[i];
		
	}

	Log(std::endl;
	

	//Log("'"<<rawstr()<<"'");
	//
#endif

	delete [] data;
}


UStr::UStr(const UStr& original)
{
	data = new unsigned int[1];
	data[0] = 0;
	length = 0;
	*this = original;
}

UStr::UStr(const char* cstr)
{
#ifdef USTR_DEBUG
	Log("UStr::UStr(const char* cstr)");
	
#endif
	
#if 0
	length = strlen(cstr);
	data = new unsigned int [length+1];
	for(int i=0; i<length+1; i++)
		data[i] = (unsigned char)cstr[i];
#else
	data = ToUTF32((unsigned char*)cstr);
	for(length=0; data[length]; length++)
		;
#endif
}

UStr::UStr(unsigned int k)
{
	length = 1;
	data = new unsigned int [length+1];
	data[0] = k;
	data[1] = 0;
}

UStr::UStr(unsigned int* k)
{
	if(!k)
	{
		length = 0;
		data = new unsigned int [length+1];
		data[0] = 0;
		return;
	}

	for(length=0; k[length]; length++)
		;
	data = new unsigned int [length+1];
	memcpy(data, k, sizeof(unsigned int) * (length+1));
}

//#define USTR_DEBUG

UStr& UStr::operator=(const UStr& original)
{
#ifdef USTR_DEBUG
	Log("UStr= ["<<rawstr()<<"] => ["<<original.rawstr()<<"]");
	
#endif

	delete [] data;

	length = original.length;
	data = new unsigned int [length+1];
	memcpy(data, original.data, sizeof(unsigned int) * (length+1));

	return *this;
}

UStr UStr::operator+(const UStr &other)
{
	UStr newstr;

	delete [] newstr.data;

	newstr.length = length + other.length;

	newstr.data = new unsigned int[newstr.length+1];

	for(int i=0; i<length; i++)
		newstr.data[i] = data[i];

	for(int i=0; i<other.length; i++)
		newstr.data[i+length] = other.data[i];

	newstr.data[length+other.length] = 0;

	return newstr;
}

UStr UStr::substr(int start, int len) const
{
	UStr newstr;

	if(len <= 0)
		return newstr;

	//important fix
	if(start < 0 || start >= length)
		return newstr;

	delete [] newstr.data;
	newstr.length = len;
	newstr.data = new unsigned int[len+1];
	for(int i=0; i<len; i++)
		newstr.data[i] = data[start+i];

	newstr.data[len] = 0;

#ifdef USTR_DEBUG
	Log("USt substr :: "<<newstr.rawstr());
	
#endif

	return newstr;
}

int UStr::firstof(UStr find) const
{
	for(int i=0; i<length; i++)
	{
		ecbool found = ectrue;

		for(int j=0; j<find.length; j++)
		{
			if(data[i+j] != find.data[j])
			{
				found = ecfalse;
				break;
			}
		}

		if(found)
			return i;
	}

	return -1;
}

std::string UStr::rawstr() const
{
	std::string finstr;

//#ifdef USTR_DEBUG
#if 0
	Log("\t\tstring UStr::rawstr() const...");
	

	Log("\t\t\t");
	

	for(int i=0; i<length; i++)
	{
		Log("#"<<i;
		Log("'"<<(char)data[i]<<"' ";
		
	}

#endif

#if 0
	for(int i=0; i<length; i++)
		finstr += (char)data[i];
#else
	//utf8
	unsigned char* utf8 = ToUTF8(data);
	finstr = (char*)utf8;
	delete [] utf8;
#endif

	return finstr;
}


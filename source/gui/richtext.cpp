







#include "richtext.h"
#include "icon.h"
#include "../utils.h"


//#define USTR_DEBUG

RichPart::RichPart()
{
	type = RICH_TEXT;

#ifdef USTR_DEBUG
	Log("RichPart::RichPart()");
	
#endif
}

RichPart::RichPart(const RichPart& original)
{

#ifdef USTR_DEBUG
	Log("RichPart::RichPart(const RichPart& original)");
	
#endif

	*this = original;
}

RichPart::RichPart(const char* cstr)
{
	type = RICH_TEXT;
	text = UStr(cstr);

#ifdef USTR_DEBUG
	Log("RichPart::RichPart(const char* cstr) end '"<<text.rawstr()<<"'");
	
#endif
}

RichPart::RichPart(UStr ustr)
{
	type = RICH_TEXT;
	text = ustr;
}


RichPart::RichPart(int type, int subtype)
{
	if(type == RICH_ICON)
	{
		type = RICH_ICON;
		icon = subtype;
		return;
	}

	type = RICH_TEXT;
}

RichPart& RichPart::operator=(const RichPart &original)
{
#ifdef USTR_DEBUG
	Log("RichPart& RichPart::operator=(const RichPart &original)");
	
#endif

	type = original.type;
	text = original.text;
	icon = original.icon;

	return *this;
}

int RichPart::texlen() const	// icons count as 1 glyph
{
	if(type == RICH_TEXT)
		return text.length;
	else if(type == RICH_ICON)
		//return g_icon[icon].tag.length;
		return 1;

	return 0;
}

int RichPart::rawlen() const	// icon tag length is counted
{
	if(type == RICH_TEXT)
		return text.length;
	else if(type == RICH_ICON)
		return g_icon[icon].tag.length;

	return 0;
}

std::string RichPart::texval() const
{
	if(type == RICH_TEXT)
	{
#if 0
//#ifdef USTR_DEBUG
		Log("\tstring RichPart::texval() const...");
		
#endif

		return text.rawstr();

#if 0
//#ifdef USTR_DEBUG
		Log("\tstring RichPart::texval() const = "<<text.rawstr());
		
#endif
	}
	else if(type == RICH_ICON)
	{
		Icon* icon = &g_icon[icon];
		return icon->tag.rawstr();
	}

	return text.rawstr();
}

RichPart RichPart::substr(int start, int length) const
{
	if(type == RICH_ICON)
		return *this;
	else if(type == RICH_TEXT)
	{
		RichPart subp(text.substr(start, length));
		return subp;
	}

	return *this;
}

RichText::RichText(const RichPart& part)
{
	part.clear();
	part.push_back(part);
}

RichText::RichText(const RichText& original)
{
#ifdef USTR_DEBUG
	Log("RichText::RichText(const RichText& original) = "<<original.rawstr());
	
#endif
	
	*this = original;
}

RichText::RichText(const char* cstr)
{
#ifdef USTR_DEBUG
	Log("RichText::RichText(const char* cstr)");
	
#endif

	part.clear();
	part.push_back( RichPart(cstr) );
}

RichText& RichText::operator=(const RichText &original)
{
#ifdef USTR_DEBUG
//#if 1
	Log("richtext::= ";
	
	Log("from: ";
	
	Log(rawstr());
	
	Log("to: ";
	
	Log(original.rawstr());
	
#endif

	part.clear();

	for(std::list<RichPart>::const_iterator i=original.part.begin(); i!=original.part.end(); i++)
		part.push_back( *i );

	return *this;
}

RichText RichText::operator+(const RichText &other)
{
	RichPart twopart;
	ecbool havecombomid = ecfalse;

	RichPart* last1 = NULL;
	RichPart* first2 = NULL;

	//if there's something in text 1 and text 2...
	if(part.size() > 0 && other.part.size() > 0)
	{
		//get the last RichPart in text1 that has something in it
		for(std::list<RichPart>::reverse_iterator i=part.rbegin(); i!=part.rend(); i++)
		{
			if(i->texlen() > 0)
			{
				last1 = &*i;
				break;
			}
		}

		//get the first RichPart in text2 that has something in it
		for(std::list<RichPart>::const_iterator i=other.part.begin(); i!=other.part.end(); i++)
		{
			if(i->texlen() > 0)
			{
				first2 = (RichPart*)&*i;
				break;
			}
		}

		/*
		if the last RichPart of text1 and the first RichPart of text2
		are text (and not icons), they can be joined into one RichPart.
		*/
		if(last1 && 
			first2 &&
			last1->type == RICH_TEXT && 
			first2->type == RICH_TEXT)
		{
			twopart.type = RICH_TEXT;
			twopart.text = last1->text + first2->text;
			havecombomid = ectrue;
		}
		//otherwise, set these to NULL to remember not to combine them
		else
		{
			last1 = NULL;
			first2 = NULL;
		}
	}

	RichText combined;

	for(std::list<RichPart>::iterator i=part.begin(); i!=part.end(); i++)
	{
		//if this is the part that we've combined, don't add it
		if(&*i == last1)
			break;

		//don't copy if empty
		if(i->texlen() <= 0)
			continue;

		combined.part.push_back(*i);

		//Log("combined1 rawstr = "<<combined.rawstr());
	}

	//if we have a combined middle, add that
	if(twopart.texlen() > 0 && havecombomid)
		combined.part.push_back(twopart);

	//Log("combined2 rawstr = "<<combined.rawstr());

	for(std::list<RichPart>::const_iterator i=other.part.begin(); i!=other.part.end(); i++)
	{
		//if this is the part that we've combined, don't add it
		if(&*i == first2)
			//break;
			continue;	//corpc fix
		
		//don't copy if empty
		if(i->texlen() <= 0)
			continue;

		combined.part.push_back(*i);

		//Log("combined3 rawstr = "<<combined.rawstr());
	}

	return combined;
}

RichText RichText::substr(int start, int length) const
{
	RichText retval;

	int totallen = texlen();
	int currplace = 0;
	std::list<RichPart>::const_iterator currp = part.begin();

	while(currplace < totallen && currplace < start+length && currp != part.end())
	{
		int currlen = currp->texlen();

		if(currlen <= 0)
			continue;

		int startplace = start - currplace;
		int endplace = (start+length) - currplace;

		//if(currplace < start+length && currplace+currlen >= start)
		if(startplace < currlen && endplace >= 0)
		{
			RichPart addp;

			if(startplace < 0)
				startplace = 0;

			if(endplace > currlen)
				endplace = currlen;

			int addlen = endplace - startplace;

			addp = currp->substr(startplace, addlen);

			retval = retval + addp;
		}

		currplace += currlen;
		currp++;
	}

	return retval;
}

std::string RichText::rawstr() const
{
	std::string raws;

#ifdef USTR_DEBUG
	//int parti = 0;
	//Log("std::string RichText::rawstr() const before loop..."<<parti);
	//
#endif

	for(std::list<RichPart>::const_iterator i=part.begin(); i!=part.end(); i++)
	{
#ifdef USTR_DEBUG
		//Log("std::string RichText::rawstr() const parti="<<parti);
		//
		//Log("\tstring RichText::rawstr() const = "<<i->texval());
		//
		//parti++;
#endif

		raws += i->texval();
	}

	return raws;
}

int RichText::texlen() const	// icons count as 1 glyph
{
	int runl = 0;

	for(std::list<RichPart>::const_iterator i=part.begin(); i!=part.end(); i++)
		runl += i->texlen();

	return runl;
}

int RichText::rawlen() const	// icon tag length is counted
{
	int runl = 0;

	for(std::list<RichPart>::const_iterator i=part.begin(); i!=part.end(); i++)
		runl += i->rawlen();

	return runl;
}

RichText::RichText()
{
}

RichText RichText::pwver() const	//asterisk-mask password std::string
{
	int len = 0;

	for(std::list<RichPart>::const_iterator i=part.begin(); i!=part.end(); i++)
		len += i->texlen();

	char* pwcstr = new char[len+1];

	for(int i=0; i<len; i++)
		pwcstr[i] = '*';

	pwcstr[len] = '\0';

	RichPart pwstrp(pwcstr);
	delete pwcstr;

	RichText pwstr;
	pwstr.part.push_back(pwstrp);

	return pwstr;
}

#ifdef USTR_DEBUG
int parsedepth = 0;
#endif

RichText ParseTags(RichText original, int* caret)
{
	RichText parsed;
	int currplace = 0;

	ecbool changed = ecfalse;

#ifdef USTR_DEBUG
	parsedepth ++;

	Log("ParseTags #"<<parsedepth);
	

	//if(parsedepth > 10)
	//	return original;
#endif
	
	for(std::list<RichPart>::iterator i=original.part.begin(); i!=original.part.end(); i++)
	{
		if(i->type == RICH_TEXT)
		{
			ecbool foundtag = ecfalse;

			std::string num;
			int firstof = -1;
			int hashoff = -1;
			int lastof = -1;

			RichPart* p = &*i;
			UStr* s = &p->text;
			unsigned int* u = s->data;

			for(int j=0; j<s->length; j++)
			{
				if(u[j] == '&' &&
					!foundtag)
				{
					firstof = j;
					lastof = j;
					num.clear();
				}
				else if(u[j] == '#' &&
					!foundtag &&
					firstof == j-1 &&
					firstof >= 0)
				{
					hashoff = j;
					lastof = j;
					num.clear();
				}
				else if(u[j] == ';' && 
					firstof >= 0 && 
					hashoff == firstof+1 &&
					!foundtag && 
					lastof > firstof &&
					num.length() > 0)
				{
					lastof = j;
					foundtag = ectrue;
				}
				else if(u[j] >= '0' &&
					u[j] <= '9' &&
					firstof >= 0 &&
					hashoff == firstof+1 &&
					!foundtag)
				{
					num += (char)u[j];
				}
				else if(!foundtag)
				{
					num.clear();
					firstof = -1;
					hashoff = -1;
					lastof = -1;
				}
			}

			if(!foundtag)
			{
				parsed = parsed + *i;
				continue;
			}

#ifdef USTR_DEBUG
			Log("ParseTags found tag \""<<icon->tag.rawstr()<<"\" in \""<<i->text.rawstr()<<"\"");
			
#endif

			if(firstof > 0)
			{
				RichPart before = i->substr(0, firstof);

#ifdef USTR_DEBUG
				Log("ParseTags before str at "<<firstof<<" \""<<before.text.rawstr()<<"\"");
				
#endif

				parsed = parsed + RichText(before);

#ifdef USTR_DEBUG
				Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
				
#endif
			}

			unsigned int addi = StrToInt(num.c_str());
			RichPart addp(addi);
			parsed = parsed + RichText(addp);

			int taglen = lastof - firstof + 1;
			int partlen =  i->text.length;

#ifdef USTR_DEBUG
			Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
			
#endif

			if(firstof+taglen < partlen)
			{
				RichPart after = i->substr(firstof+taglen, partlen-(firstof+taglen));

#ifdef USTR_DEBUG
				Log("ParseTags after str at "<<(firstof+taglen)<<" \""<<after.text.rawstr()<<"\"");
				
#endif

				parsed = parsed + RichText(after);

#ifdef USTR_DEBUG
				Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
				
#endif
			}

			if(caret != NULL)
			{
				if(currplace+firstof < *caret)
				{
					*caret -= taglen-1;
					currplace += partlen-taglen+1;
				}
			}

			foundtag = ectrue;
			changed = ectrue;
		}
		else
		{
			parsed = parsed + *i;
		}

		if(!changed && caret != NULL)
			currplace += i->texlen();
	}

	if(changed)
		return ParseTags(parsed, caret);

	//reset
	parsed = RichText("");

	for(std::list<RichPart>::iterator i=original.part.begin(); i!=original.part.end(); i++)
	{
		if(i->type == RICH_TEXT)
		{
			ecbool foundtag = ecfalse;

			for(int j=0; j<ICONS; j++)
			{
				Icon* icon = &g_icon[j];
				int firstof = i->text.firstof(icon->tag);

				if(firstof < 0)
					continue;

#ifdef USTR_DEBUG
				Log("ParseTags found tag \""<<icon->tag.rawstr()<<"\" in \""<<i->text.rawstr()<<"\"");
				
#endif

				if(firstof > 0)
				{
					RichPart before = i->substr(0, firstof);

#ifdef USTR_DEBUG
					Log("ParseTags before str at "<<firstof<<" \""<<before.text.rawstr()<<"\"");
					
#endif

					parsed = parsed + RichText(before);

#ifdef USTR_DEBUG
					Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
					
#endif
				}

				RichPart iconp(RICH_ICON, j);
				parsed = parsed + RichText(iconp);

				int taglen = icon->tag.length;
				int partlen =  i->text.length;

#ifdef USTR_DEBUG
				Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
				
#endif

				if(firstof+taglen < partlen)
				{
					RichPart after = i->substr(firstof+taglen, partlen-(firstof+taglen));

#ifdef USTR_DEBUG
					Log("ParseTags after str at "<<(firstof+taglen)<<" \""<<after.text.rawstr()<<"\"");
					
#endif

					parsed = parsed + RichText(after);

#ifdef USTR_DEBUG
					Log("\tparsed now = \""<<parsed.rawstr()<<"\"");
					
#endif
				}

				if(caret != NULL)
				{
					if(currplace+firstof < *caret)
					{
						*caret -= taglen-1;
						currplace += partlen-taglen+1;
					}
				}

				foundtag = ectrue;
				changed = ectrue;

				break;
			}

			if(!foundtag)
			{
				parsed = parsed + *i;
			}
		}
		else
		{
			parsed = parsed + *i;
		}

		if(!changed && caret != NULL)
			currplace += i->texlen();
	}
	
	if(changed)
		return ParseTags(parsed, caret);

#ifdef USTR_DEBUG
	Log("ParseTags final = "<<original.rawstr());
	
#endif

	return original;
}

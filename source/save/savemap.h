











#ifndef SAVEMAP_H
#define SAVEMAP_H

#define MAP_TAG			{'S','F','M'}
#define MAP_VERSION		25

float ConvertHeight(unsigned char brightness);
void FreeMap();
ecbool LoadMap(const char* relative, ecbool live=ecfalse, ecbool afterdown=ecfalse);
ecbool SaveMap(const char* relative);

#endif

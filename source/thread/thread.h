

#ifndef THREAD_H
#define THREAD_H

#include "../platform.h"

struct Thread
{
public:
	ecbool quit;
	SDL_mutex* mutex;
	SDL_Thread* handle;
};

#define THREADS	8

extern Thread g_thread[THREADS];

int ThreadFun(void* param);

#endif